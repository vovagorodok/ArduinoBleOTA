#pragma once

#if defined(BLE_OTA_STORAGE_LIB_ARDUINO_OTA)
    #define BLE_OTA_STORAGE_LIB_PREDEFINED
#elif defined(BLE_OTA_STORAGE_LIB_INTERNAL)
    #define BLE_OTA_STORAGE_LIB_PREDEFINED
#endif

#if !defined(PLATFORMIO) && !defined(BLE_OTA_STORAGE_LIB_PREDEFINED)
    #define BLE_OTA_STORAGE_LIB_ARDUINO_OTA
    #define BLE_OTA_STORAGE_LIB_PREDEFINED
#endif

#if !defined(BLE_OTA_STORAGE_LIB_PREDEFINED)
    #if __has_include("ArduinoOTA.h") && __has_include("OTAStorage.h")
        #define BLE_OTA_STORAGE_LIB_ARDUINO_OTA
    #else
        #define BLE_OTA_STORAGE_LIB_INTERNAL
    #endif
#endif

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
#elif defined(BLE_OTA_STORAGE_LIB_INTERNAL)
    #if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
        #include "BleOtaInternalStorageESP.h"
    #else
        #error "Unsupported storage library. Consider ArduinoOTA."
    #endif
#else
    #error "Unsupported storage library. Consider ArduinoOTA."
#endif