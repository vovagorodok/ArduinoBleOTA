#include "ArduinoBleOTA.h"
#include <iostream>

namespace
{
#define OTA_SERVICE_UUID           "c68680a2-d922-11ec-bd40-7ff604147105"
#define OTA_CHARACTERISTIC_UUID_RX "c6868174-d922-11ec-bd41-c71bb0ce905a"
#define OTA_CHARACTERISTIC_UUID_TX "c6868246-d922-11ec-bd42-7b10244d223f"
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
    if (contentLength == 0)
        onStartUpdate(characteristic);
    else
        onUpdate(characteristic);
}

void ArduinoBleOTAClass::onStartUpdate(BLECharacteristic* characteristic)
{
    long contentLength = characteristic->getValue<long>();

    if (contentLength <= 0) {
        send("Incorrect length");
        return;
    }

    if (storage == NULL || !storage->open(contentLength)) {
        send("Storage error");
        return;
    }

    if (storage->maxSize() && contentLength > storage->maxSize()) {
        storage->close();
        send("Payload too large");
        return;
    }

    this->contentLength = contentLength;
    this->contentUploaded = 0;
}

void ArduinoBleOTAClass::onUpdate(BLECharacteristic* characteristic)
{
    auto data = characteristic->getValue<uint8_t*>();
    auto length = characteristic->getDataLength();
    contentUploaded += length;

    if (contentUploaded > contentLength)
    {
        storage->clear();
        contentUploaded = contentLength = 0;
        send("Payload size wrong");
        delay(500);
        return;
    }

    for (size_t i = 0; i < length; i++)
        storage->write(data[i]);

    if (contentUploaded == contentLength)
    {
        send("OK");
        delay(500);
        storage->apply();
        while (true);
    }
}

void ArduinoBleOTAClass::send(const std::string& str)
{
    txCharacteristic->setValue(str);
    txCharacteristic->notify();
}