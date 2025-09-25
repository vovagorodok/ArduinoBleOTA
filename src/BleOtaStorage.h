#pragma once
#include "BleOtaStatus.h"
#include "BleOtaStorageDefines.h"

#if defined(BLE_OTA_STORAGE_LIB_ARDUINO_OTA)
    // Copied from https://github.com/JAndrassy/ArduinoOTA/blob/master/src/ArduinoOTA.h
    #ifdef __AVR__
    #if FLASHEND >= 0xFFFF
    #include "InternalStorageAVR.h"
    #endif
    #elif defined(ARDUINO_ARCH_STM32)
    #include <InternalStorageSTM32.h>
    #elif defined(ARDUINO_ARCH_RP2040)
    #include <InternalStorageRP2.h>
    #elif defined(ARDUINO_ARCH_RENESAS_UNO)
    #include <InternalStorageRenesas.h>
    #elif defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
    #include "InternalStorageESP.h"
    #else
    #include "InternalStorage.h"
    #endif
    #ifdef __SD_H__
    #include "SDStorage.h"
    SDStorageClass SDStorage;
    #endif
    #ifdef SerialFlash_h_
    #include "SerialFlashStorage.h"
    SerialFlashStorageClass SerialFlashStorage;
    #endif
#elif defined(BLE_OTA_STORAGE_LIB_INTERNAL_ESP)
    #include "BleOtaInternalStorageESP.h"
#else
    #error "Unsupported storage library. Consider ArduinoOTA."
#endif

class BleOtaStorage
{
public:
    BleOtaStorage();

    void begin(OTAStorage& storage);
    BleOtaStatus open(size_t firmwareSize);
    BleOtaStatus push(const uint8_t* data, size_t size);
    void close();
    void apply();
    void clear();
    bool isFull() const;

private:
    OTAStorage* _storage;
    size_t _firmwareSize;
    size_t _size;
};