#include "ArduinoBleOTA.h"
#include <iostream>

namespace
{
#define OTA_SERVICE_UUID           "c68680a2-d922-11ec-bd40-7ff604147105"
#define OTA_CHARACTERISTIC_UUID_RX "c6868174-d922-11ec-bd41-c71bb0ce905a"
#define OTA_CHARACTERISTIC_UUID_TX "c6868246-d922-11ec-bd42-7b10244d223f"

#define OK 0x00
#define NOK 0x01
#define INCORRECT_FORMAT 0x02
#define INCORRECT_FIRMWARE_SIZE 0x03
#define CHECKSUM_ERROR 0x04
#define INTERNAL_STORAGE_ERROR 0x05

#define BEGIN 0x10
#define PACKAGE 0x11
#define END 0x12
}

void ArduinoBleOTAClass::begin(const std::string &deviceName, OTAStorage& storage)
{
    BLEDevice::init(deviceName);
    auto* server = BLEDevice::createServer();

    begin(server, storage);

    auto* advertising = server->getAdvertising();
    advertising->setScanResponse(true);
    advertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    advertising->setMaxPreferred(0x12);
    advertising->start();
}

void ArduinoBleOTAClass::begin(NimBLEServer* server, OTAStorage& storage)
{
    begin(server,
          storage,
          OTA_SERVICE_UUID,
          OTA_CHARACTERISTIC_UUID_RX,
          OTA_CHARACTERISTIC_UUID_TX);
}

void ArduinoBleOTAClass::begin(
    NimBLEServer* server,
    OTAStorage& storage,
    const char* serviceUUID,
    const char* rxChUUID,
    const char* txChUUID)
{
    this->storage = &storage;
    auto* service = server->createService(serviceUUID);

    auto* rxCharacteristic = service->createCharacteristic(
        rxChUUID,
        NIMBLE_PROPERTY::WRITE
    );
    rxCharacteristic->setCallbacks(this);

    auto* txCharacteristic = service->createCharacteristic(
        txChUUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    this->txCharacteristic = txCharacteristic;

    service->start();
    auto* advertising = server->getAdvertising();
    advertising->addServiceUUID(serviceUUID);
}

void ArduinoBleOTAClass::onWrite(BLECharacteristic* characteristic)
{
    auto data = characteristic->getValue<uint8_t*>();
    auto length = characteristic->getDataLength();

    if (length == 0)
    {
        send(INCORRECT_FORMAT);
        return;
    }

    switch (data[0])
    {
    case BEGIN:
        beginUpdate(data + 1, length - 1);
        break;
    case PACKAGE:
        handlePackage(data + 1, length - 1);
        break;
    case END:
        endUpdate(data + 1, length - 1);
        break;
    default:
        send(INCORRECT_FORMAT);
        break;
    }
}

void ArduinoBleOTAClass::beginUpdate(uint8_t* data, size_t length)
{
    if (updating)
        stopUpdate();

    if (length != sizeof(uint32_t))
    {
        send(INCORRECT_FORMAT);
        return;
    }
    firmwareLength = *reinterpret_cast<uint32_t*>(data);

    if (storage == nullptr or not storage->open(firmwareLength))
    {
        firmwareLength = 0;
        send(INTERNAL_STORAGE_ERROR);
        return;
    }

    if (storage->maxSize() and currentLength > storage->maxSize())
    {
        send(INCORRECT_FIRMWARE_SIZE);
        stopUpdate();
        return;
    }

    currentLength = 0;
    updating = true;
    crc.reset();
    send(OK);
}

void ArduinoBleOTAClass::handlePackage(uint8_t* data, size_t length)
{
    if (not updating)
    {
        send(NOK);
        return;
    }

    currentLength += length;

    if (currentLength > firmwareLength)
    {
        send(INCORRECT_FIRMWARE_SIZE);
        stopUpdate();
        return;
    }

    for (size_t i = 0; i < length; i++)
    {
        storage->write(data[i]);
        crc.update(data[i]);
    }

    send(OK);
}

void ArduinoBleOTAClass::endUpdate(uint8_t* data, size_t length)
{
    if (not updating)
    {
        send(NOK);
        return;
    }
    if (currentLength != firmwareLength)
    {
        send(INCORRECT_FIRMWARE_SIZE);
        stopUpdate();
        return;
    }
    if (length != sizeof(uint32_t))
    {
        send(INCORRECT_FORMAT);
        return;
    }
    auto firmwareCrc = *reinterpret_cast<uint32_t*>(data);

    if (crc.finalize() != firmwareCrc)
    {
        send(CHECKSUM_ERROR);
        stopUpdate();
        return;
    }

    send(OK);
    delay(500);
    storage->apply();
    while (true);
}

void ArduinoBleOTAClass::send(uint8_t head)
{
    txCharacteristic->setValue(&head, 1);
    txCharacteristic->notify();
}

void ArduinoBleOTAClass::stopUpdate()
{
    storage->clear();
    storage->close();
    updating = false;
    currentLength = firmwareLength = 0;
}