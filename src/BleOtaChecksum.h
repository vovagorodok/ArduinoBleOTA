#pragma once
#include "BleOtaDefinesArduino.h"

#if defined(BLE_OTA_CHECKSUM_LIB_MINIZ)
    #include <miniz.h>
    #define BLE_OTA_CHECKSUM_LIB_PREDEFINED
#elif defined(BLE_OTA_CHECKSUM_LIB_CRC)
    #include <CRC32.h>
    #define BLE_OTA_CHECKSUM_LIB_PREDEFINED
#elif defined(BLE_OTA_NO_CHECKSUM)
    #define BLE_OTA_CHECKSUM_LIB_PREDEFINED
#endif

#if !defined(PLATFORMIO) && !defined(BLE_OTA_CHECKSUM_LIB_PREDEFINED)
    #define BLE_OTA_NO_CHECKSUM
    #define BLE_OTA_CHECKSUM_LIB_PREDEFINED
#endif

#if !defined(BLE_OTA_CHECKSUM_LIB_PREDEFINED)
    #if defined(BLE_OTA_CHECKSUM_LIB_CRC)
        #include <CRC32.h>
    #elif __has_include("miniz.h")
        #include <miniz.h>
        #define BLE_OTA_CHECKSUM_LIB_MINIZ
    #elif __has_include("rom/miniz.h")
        #include <rom/miniz.h>
        #define BLE_OTA_CHECKSUM_LIB_MINIZ
    #elif __has_include("CRC32.h")
        #include <CRC32.h>
        #define BLE_OTA_CHECKSUM_LIB_CRC
    #else
        #define BLE_OTA_NO_CHECKSUM
    #endif
#endif

class BleOtaChecksum
{
public:
    BleOtaChecksum();

    void begin();
    void push(const uint8_t* data, size_t size);
    uint32_t calc() const;
    void setEnable(bool enable);
    bool isEnabled() const;
    bool isSupported() const;

private:
#if defined(BLE_OTA_CHECKSUM_LIB_CRC)
    CRC32 _crc;
#elif defined(BLE_OTA_CHECKSUM_LIB_MINIZ)
    mz_ulong _crc = 0;
#endif
#ifndef BLE_OTA_NO_CHECKSUM
    bool _enable;
#endif
};