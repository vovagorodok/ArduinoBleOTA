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
#define INSTALL 0x13
}

bool ArduinoBleOTAClass::begin(const std::string &deviceName, OTAStorage& storage)
{
    BLEDevice::init(deviceName);
    auto* server = BLEDevice::createServer();

    if(!begin(server, storage))
        return false;

    auto* advertising = server->getAdvertising();
    advertising->setScanResponse(true);
    advertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    advertising->setMaxPreferred(0x12);
    return advertising->start();
}

bool ArduinoBleOTAClass::begin(NimBLEServer* server, OTAStorage& storage)
{
    return begin(server,
                 storage,
                 OTA_SERVICE_UUID,
                 OTA_CHARACTERISTIC_UUID_RX,
                 OTA_CHARACTERISTIC_UUID_TX);
}

bool ArduinoBleOTAClass::begin(
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

    auto* advertising = server->getAdvertising();
    advertising->addServiceUUID(serviceUUID);
    return service->start();
}

void ArduinoBleOTAClass::onWrite(BLECharacteristic* characteristic)
{
    auto value = characteristic->getValue();
    auto data = value.data();
    auto length = value.length();

    if (length == 0)
    {
        send(INCORRECT_FORMAT);
        return;
    }

    switch (data[0])
    {
    case BEGIN:
        handleBegin(data + 1, length - 1);
        break;
    case PACKAGE:
        handlePackage(data + 1, length - 1);
        break;
    case END:
        handleEnd(data + 1, length - 1);
        break;
    case INSTALL:
        handleInstall();
        break;
    default:
        send(INCORRECT_FORMAT);
        break;
    }
}

void ArduinoBleOTAClass::handleBegin(const uint8_t* data, size_t length)
{
    if (updating)
        stopUpdate();

    if (length != sizeof(uint32_t))
    {
        send(INCORRECT_FORMAT);
        return;
    }
    firmwareLength = *reinterpret_cast<const uint32_t*>(data);

    if (storage == nullptr or not storage->open(firmwareLength))
    {
        firmwareLength = 0;
        send(INTERNAL_STORAGE_ERROR);
        return;
    }

    if (storage->maxSize() and currentLength > storage->maxSize())
    {
        stopUpdate();
        send(INCORRECT_FIRMWARE_SIZE);
        return;
    }

    currentLength = 0;
    updating = true;
    crc.reset();
    send(OK);
}

void ArduinoBleOTAClass::handlePackage(const uint8_t* data, size_t length)
{
    if (not updating)
    {
        send(NOK);
        return;
    }

    currentLength += length;

    if (currentLength > firmwareLength)
    {
        stopUpdate();
        send(INCORRECT_FIRMWARE_SIZE);
        return;
    }

    for (size_t i = 0; i < length; i++)
    {
        storage->write(data[i]);
        crc.update(data[i]);
    }

    send(OK);
}

void ArduinoBleOTAClass::handleEnd(const uint8_t* data, size_t length)
{
    if (not updating)
    {
        send(NOK);
        return;
    }
    if (currentLength != firmwareLength)
    {
        stopUpdate();
        send(INCORRECT_FIRMWARE_SIZE);
        return;
    }
    if (length != sizeof(uint32_t))
    {
        send(INCORRECT_FORMAT);
        return;
    }
    auto firmwareCrc = *reinterpret_cast<const uint32_t*>(data);

    if (crc.finalize() != firmwareCrc)
    {
        stopUpdate();
        send(CHECKSUM_ERROR);
        return;
    }

    send(OK);
}

void ArduinoBleOTAClass::handleInstall()
{
    storage->close();
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