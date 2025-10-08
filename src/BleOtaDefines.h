#pragma once

#if __has_include("ArduinoBLE.h")
    #include <ArduinoBLE.h>
    #define BLE_OTA_BLE_LIB_ARDUINO_BLE
    #define BLE_OTA_LIB_ARDUINO_BLE
#elif __has_include("NimBLEDevice.h")
    #include <NimBLEDevice.h>
    #define BLE_OTA_BLE_LIB_NIM_BLE_ARDUINO
    #define BLE_OTA_LIB_NIM_BLE_ARDUINO
#if !defined(CONFIG_NIMBLE_MAX_CONNECTIONS) && \
    !defined(CONFIG_NIMBLE_CPP_DEBUG_ASSERT_ENABLED)
    #define BLE_OTA_BLE_LIB_NIM_BLE_ARDUINO_V1
#else
    #define BLE_OTA_BLE_LIB_NIM_BLE_ARDUINO_V2
#endif
#elif defined(ARDUINO_ARCH_ESP32)
    #include <BLEDevice.h>
    #define BLE_OTA_BLE_LIB_ESP32
    #define BLE_OTA_LIB_ESP32
#else
    #error "Unsupported BLE library. Consider ArduinoBLE or NimBLE-Arduino."
#endif