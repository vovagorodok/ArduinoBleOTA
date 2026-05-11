#pragma once
#include "BleOtaDefinesArduino.h"

// clang-format off: Used IndentPPDirectives with BeforeHash
#ifdef BLE_OTA_LOGS
    #if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
        #define BLE_OTA_PRINTF Serial.printf
    #else
        #define BLE_OTA_PRINTF printf
    #endif

    #define BLE_OTA_LOG(tag, fmt, ...) \
        BLE_OTA_PRINTF("DBG: BleOta: " tag ": " fmt "\n", ##__VA_ARGS__)
#else
    #define BLE_OTA_LOG(fmt, ...) \
        do {} while (0)
#endif
// clang-format on
