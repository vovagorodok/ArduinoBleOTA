#include "BleOtaUploader.h"
#include "ArduinoBleOTA.h"
#include "BleOtaUtils.h"

namespace
{
#define OK 0x00
#define NOK 0x01
#define INCORRECT_FORMAT 0x02
#define INCORRECT_FIRMWARE_SIZE 0x03
#define CHECKSUM_ERROR 0x04
#define INTERNAL_STORAGE_ERROR 0x05

#define BEGIN 0x10
#define PACKAGE 0x11
#define END 0x12

#pragma pack(push, 1)
struct BeginResponse
{
    uint8_t head;
    uint32_t attributeSize;
    uint32_t bufferSize;
};
#pragma pack(pop)
}

void BleOtaUploader::begin(OTAStorage& storage)
{
    this->storage = &storage;
}

void BleOtaUploader::pull()
{
    if (installing)
        handleInstall();
}

void BleOtaUploader::onData(const uint8_t* data, size_t length)
{
    if (installing)
        return;

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

void BleOtaUploader::handleBegin(const uint8_t* data, size_t length)
{
    if (uploading)
        terminateUpload();

    if (length != sizeof(uint32_t))
    {
        send(INCORRECT_FORMAT);
        return;
    }
    firmwareLength = addrToRef<uint32_t>(data);

    if (storage == nullptr or not storage->open(firmwareLength))
    {
        firmwareLength = 0;
        send(INTERNAL_STORAGE_ERROR);
        return;
    }

    if (storage->maxSize() and firmwareLength > storage->maxSize())
    {
        terminateUpload();
        send(INCORRECT_FIRMWARE_SIZE);
        return;
    }

    currentLength = 0;
    uploading = true;
    crc.reset();

    #ifndef BLE_OTA_NO_BUFFER
    buffer.clear();
    uint32_t bufferSize = withBuffer ? BLE_OTA_BUFFER_SIZE : 0;
    #else
    uint32_t bufferSize = 0;
    #endif
    BeginResponse resp{OK, BLE_OTA_ATTRIBUTE_SIZE, bufferSize};
    send(refToAddr(resp), sizeof(BeginResponse));
}

void BleOtaUploader::handlePackage(const uint8_t* data, size_t length)
{
    if (not uploading)
        return;

    currentLength += length;
    #ifndef BLE_OTA_NO_BUFFER
    const bool sendResponse = not withBuffer or buffer.size() + length > BLE_OTA_BUFFER_SIZE;
    #else
    const bool sendResponse = true;
    #endif

    if (currentLength > firmwareLength)
    {
        terminateUpload();
        if (sendResponse) send(INCORRECT_FIRMWARE_SIZE);
        return;
    }

    #ifndef BLE_OTA_NO_BUFFER
    if (sendResponse)
    {
        flushBuffer();
    }
    #endif

    fillData(data, length);
    if (sendResponse) send(OK);
}

void BleOtaUploader::handleEnd(const uint8_t* data, size_t length)
{
    if (not uploading)
    {
        send(NOK);
        return;
    }
    if (currentLength != firmwareLength)
    {
        terminateUpload();
        send(INCORRECT_FIRMWARE_SIZE);
        return;
    }
    if (length != sizeof(uint32_t))
    {
        send(INCORRECT_FORMAT);
        return;
    }
    auto firmwareCrc = addrToRef<uint32_t>(data);

    if (crc.finalize() != firmwareCrc)
    {
        terminateUpload();
        send(CHECKSUM_ERROR);
        return;
    }

    #ifndef BLE_OTA_NO_BUFFER
    flushBuffer();
    #endif

    installing = true;
    send(OK);
}

void BleOtaUploader::handleInstall()
{
    storage->close();
    delay(500);
    storage->apply();
    while (true);
}

void BleOtaUploader::send(uint8_t head)
{
    send(&head, 1);
}

void BleOtaUploader::send(const uint8_t* data, size_t length)
{
    ArduinoBleOTA.send(data, length);
}

void BleOtaUploader::terminateUpload()
{
    storage->clear();
    storage->close();
    uploading = false;
    currentLength = firmwareLength = 0;

    #ifndef BLE_OTA_NO_BUFFER
    withBuffer = false;
    #endif
}

void BleOtaUploader::fillData(const uint8_t* data, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        #ifndef BLE_OTA_NO_BUFFER
        withBuffer ? buffer.push(data[i]) : storage->write(data[i]);
        #else
        storage->write(data[i]);
        #endif
        crc.update(data[i]);
    }
}

#ifndef BLE_OTA_NO_BUFFER
void BleOtaUploader::flushBuffer()
{
    while (not buffer.isEmpty())
    {
        storage->write(buffer.shift());
    }
}
#endif

BleOtaUploader bleOtaUploader{};