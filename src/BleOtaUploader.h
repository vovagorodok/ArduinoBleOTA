#pragma once
#include "BleOtaSizes.h"
#include "BleOtaStorage.h"
#include <CRC32.h>

#ifndef BLE_OTA_NO_BUFFER
#include <CircularBuffer.hpp>
#endif

class BleOtaUploader
{
public:
    BleOtaUploader();

    void begin(OTAStorage& storage);
    void pull();
    void setEnabling(bool enabling);
    void onData(const uint8_t* data, size_t length);

private:
    void handleBegin(const uint8_t* data, size_t length);
    void handlePackage(const uint8_t* data, size_t length);
    void handleEnd(const uint8_t* data, size_t length);
    void handleSetPin(const uint8_t* data, size_t length);
    void handleRemovePin(const uint8_t* data, size_t length);
    void handleInstall();
    void handleError(uint8_t errorCode);
    void send(uint8_t head);
    void send(const uint8_t* data, size_t length);
    void terminateUpload();
    void fillData(const uint8_t* data, size_t length);
#ifndef BLE_OTA_NO_BUFFER
    void flushBuffer();
#endif

    CRC32 crc;
    OTAStorage* storage;
#ifndef BLE_OTA_NO_BUFFER
    CircularBuffer<uint8_t, BLE_OTA_BUFFER_SIZE> buffer;
    bool withBuffer;
#endif
    bool enabled;
    bool uploading;
    bool installing;
    uint32_t firmwareLength;
};

extern BleOtaUploader bleOtaUploader;