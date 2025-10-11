#pragma once
#include "BleOtaStatus.h"
#include <Arduino.h>

#if defined(BLE_OTA_COMPRESSION_LIB_MINIZ)
    #include <miniz.h>
    #define BLE_OTA_COMPRESSION_LIB_PREDEFINED
#elif defined(BLE_OTA_NO_COMPRESSION)
    #define BLE_OTA_COMPRESSION_LIB_PREDEFINED
#endif

#if !defined(PLATFORMIO) && !defined(BLE_OTA_COMPRESSION_LIB_PREDEFINED)
    #define BLE_OTA_NO_COMPRESSION
    #define BLE_OTA_COMPRESSION_LIB_PREDEFINED
#endif

#if !defined(BLE_OTA_COMPRESSION_LIB_PREDEFINED)
    #if __has_include("miniz.h")
        #include <miniz.h>
        #define BLE_OTA_COMPRESSION_LIB_MINIZ
    #elif __has_include("rom/miniz.h")
        #include <rom/miniz.h>
        #define BLE_OTA_COMPRESSION_LIB_MINIZ
    #else
        #define BLE_OTA_NO_COMPRESSION
    #endif
#endif

#if defined(BLE_OTA_STATIC_COMPRESSION) && defined(BLE_OTA_DYNAMIC_COMPRESSION)
    #error "BLE_OTA_STATIC_COMPRESSION and BLE_OTA_DYNAMIC_COMPRESSION cannot be defined at the same time."
#elif defined(BLE_OTA_STATIC_COMPRESSION) && defined(BLE_OTA_NO_COMPRESSION)
    #error "BLE_OTA_STATIC_COMPRESSION and BLE_OTA_NO_COMPRESSION cannot be defined at the same time."
#elif defined(BLE_OTA_DYNAMIC_COMPRESSION) && defined(BLE_OTA_NO_COMPRESSION)
    #error "BLE_OTA_DYNAMIC_COMPRESSION and BLE_OTA_NO_COMPRESSION cannot be defined at the same time."
#endif

#if !defined(BLE_OTA_STATIC_COMPRESSION) && !defined(BLE_OTA_DYNAMIC_COMPRESSION) && !defined(BLE_OTA_NO_COMPRESSION)
    #if defined(BLE_OTA_NO_ALLOCATION)
        #define BLE_OTA_NO_COMPRESSION
    #elif defined(BLE_OTA_STATIC_ALLOCATION)
        #define BLE_OTA_STATIC_COMPRESSION
    #elif defined(BLE_OTA_DYNAMIC_ALLOCATION)
        #define BLE_OTA_DYNAMIC_COMPRESSION
    #else
        #define BLE_OTA_DYNAMIC_COMPRESSION
    #endif
#endif

class BleOtaUploader;

class BleOtaDecompressor
{
public:
    BleOtaDecompressor(BleOtaUploader* uploader);

    void begin(size_t compressedSize);
    BleOtaStatus push(const uint8_t* data, size_t size);
    void end();
    void setEnable(bool enable);
    bool isEnabled() const;
    bool isSupported() const;

private:
    void clear();

    BleOtaUploader* _uploader;
#if defined(BLE_OTA_STATIC_COMPRESSION)
    tinfl_decompressor _decompressorData;
    uint8_t _bufferData[TINFL_LZ_DICT_SIZE];
#endif
#ifndef BLE_OTA_NO_COMPRESSION
    tinfl_decompressor* _decompressor;
    uint8_t* _buffer;
    size_t _bufferSize;
    size_t _compressedSize;
    size_t _size;
    bool _enable;
#endif
};