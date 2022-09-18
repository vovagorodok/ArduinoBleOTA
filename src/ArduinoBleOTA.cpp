#include "ArduinoBleOTA.h"
#include <iostream>

namespace
{
#define OTA_SERVICE_UUID                "15c155ca-36c5-11ed-adc0-9741d6a72f04"
#define OTA_CHARACTERISTIC_UUID_RX      "15c1564c-36c5-11ed-adc1-a3d6cf5cc2a4"
#define OTA_CHARACTERISTIC_UUID_TX      "15c156e2-36c5-11ed-adc2-7396d4fd413a"
#define OTA_CHARACTERISTIC_UUID_HW_NAME "15c1576e-36c5-11ed-adc3-8799895de51e"
#define OTA_CHARACTERISTIC_UUID_HW_VER  "15c157fa-36c5-11ed-adc4-579c60267b47"
#define OTA_CHARACTERISTIC_UUID_SW_NAME "15c15886-36c5-11ed-adc5-1bc0d0a6069d"
#define OTA_CHARACTERISTIC_UUID_SW_VER  "15c1591c-36c5-11ed-adc6-dbe9603dbf19"

#define OK 0x00
#define NOK 0x01
#define INCORRECT_FORMAT 0x02
#define INCORRECT_FIRMWARE_SIZE 0x03
#define CHECKSUM_ERROR 0x04
#define INTERNAL_STORAGE_ERROR 0x05

#define BEGIN 0x10
#define PACKAGE 0x11
#define END 0x12

constexpr auto UNKNOWN = "UNKNOWN";
}

bool ArduinoBleOTAClass::begin(const std::string &deviceName, OTAStorage& storage)
{
    return begin(deviceName, storage, UNKNOWN, {}, UNKNOWN, {});
}

bool ArduinoBleOTAClass::begin(OTAStorage& storage)
{
    return begin(storage, UNKNOWN, {}, UNKNOWN, {});
}

bool ArduinoBleOTAClass::begin(const std::string &deviceName, OTAStorage& storage,
                               const std::string &hwName, Version hwVersion,
                               const std::string &swName, Version swVersion)
{
    BLEDevice::init(deviceName);
    auto* server = BLEDevice::createServer();

    if(!begin(storage, hwName, hwVersion, swName, swVersion))
        return false;

    auto* advertising = server->getAdvertising();
    advertising->setScanResponse(true);
    advertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    advertising->setMaxPreferred(0x12);
    return advertising->start();
}

bool ArduinoBleOTAClass::begin(OTAStorage& storage,
                               const std::string &hwName, Version hwVersion,
                               const std::string &swName, Version swVersion)
{
    auto* server = BLEDevice::createServer();
    BLEDevice::setMTU(BLE_ATT_MTU_MAX);

    this->storage = &storage;
    auto* service = server->createService(OTA_SERVICE_UUID);

    auto* rxCharacteristic = service->createCharacteristic(
        OTA_CHARACTERISTIC_UUID_RX,
        NIMBLE_PROPERTY::WRITE
    );
    rxCharacteristic->setCallbacks(this);

    auto* txCharacteristic = service->createCharacteristic(
        OTA_CHARACTERISTIC_UUID_TX,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    this->txCharacteristic = txCharacteristic;

    begin(*service, hwName, hwVersion, swName, swVersion);

    auto* advertising = server->getAdvertising();
    advertising->addServiceUUID(OTA_SERVICE_UUID);
    return service->start();
}

void ArduinoBleOTAClass::begin(BLEService& service,
                               const std::string &hwName, Version hwVersion,
                               const std::string &swName, Version swVersion)
{
    auto* hwNameCharacteristic = service.createCharacteristic(
        OTA_CHARACTERISTIC_UUID_HW_NAME,
        NIMBLE_PROPERTY::READ
    );
    hwNameCharacteristic->setValue(hwName);
    auto* swNameCharacteristic = service.createCharacteristic(
        OTA_CHARACTERISTIC_UUID_SW_NAME,
        NIMBLE_PROPERTY::READ
    );
    swNameCharacteristic->setValue(swName);
    auto* hwVerCharacteristic = service.createCharacteristic(
        OTA_CHARACTERISTIC_UUID_HW_VER,
        NIMBLE_PROPERTY::READ
    );
    hwVerCharacteristic->setValue(reinterpret_cast<uint8_t*>(&hwVersion), sizeof(Version));
    auto* swVerCharacteristic = service.createCharacteristic(
        OTA_CHARACTERISTIC_UUID_SW_VER,
        NIMBLE_PROPERTY::READ
    );
    swVerCharacteristic->setValue(reinterpret_cast<uint8_t*>(&swVersion), sizeof(Version));
}

void ArduinoBleOTAClass::pull()
{
    if (installing)
        handleInstall();
}

void ArduinoBleOTAClass::onWrite(BLECharacteristic* characteristic)
{
    if (installing)
        return;

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
    default:
        send(INCORRECT_FORMAT);
        break;
    }
}

void ArduinoBleOTAClass::handleBegin(const uint8_t* data, size_t length)
{
    if (uploading)
        stopUpload();

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
        stopUpload();
        send(INCORRECT_FIRMWARE_SIZE);
        return;
    }

    currentLength = 0;
    uploading = true;
    crc.reset();
    send(OK);
}

void ArduinoBleOTAClass::handlePackage(const uint8_t* data, size_t length)
{
    if (not uploading)
    {
        send(NOK);
        return;
    }

    currentLength += length;

    if (currentLength > firmwareLength)
    {
        stopUpload();
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
    if (not uploading)
    {
        send(NOK);
        return;
    }
    if (currentLength != firmwareLength)
    {
        stopUpload();
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
        stopUpload();
        send(CHECKSUM_ERROR);
        return;
    }

    installing = true;
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

void ArduinoBleOTAClass::stopUpload()
{
    storage->clear();
    storage->close();
    uploading = false;
    currentLength = firmwareLength = 0;
}