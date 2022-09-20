#pragma once
#include "BleOtaSizes.h"
#include "BleOtaStorage.h"
#include <CRC32.h>

#ifndef BLE_OTA_NO_BUFFER
#include <CircularBuffer.h>
#endif

class BleOtaUploader
{
public:
    void begin(OTAStorage& storage);
    void pull();
    void onData(const uint8_t* data, size_t length);

private:
    void handleBegin(const uint8_t* data, size_t length);
    void handlePackage(const uint8_t* data, size_t length);
    void handleEnd(const uint8_t* data, size_t length);
    void handleInstall();
    void send(uint8_t head);
    void send(const uint8_t* data, size_t length);
    void terminateUpload();
    void fillData(const uint8_t* data, size_t length);
#ifndef BLE_OTA_NO_BUFFER
    void flushBuffer();
#endif
    CRC32 crc;
    OTAStorage* storage;
    uint32_t currentLength;
    uint32_t firmwareLength = 0;
    bool uploading = false;
    bool installing = false;
#ifndef BLE_OTA_NO_BUFFER
    bool withBuffer = true;
    CircularBuffer<uint8_t, BLE_OTA_BUFFER_SIZE> buffer;
#endif
};

extern BleOtaUploader bleOtaUploader;