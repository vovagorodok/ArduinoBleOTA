#pragma once

#if __has_include("ArduinoOTA.h") && __has_include("OTAStorage.h") && !defined(BLE_OTA_STORAGE_LIB_INTERNAL)
#define BLE_OTA_STORAGE_LIB_ARDUINO_OTA
#elif defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
#define BLE_OTA_STORAGE_LIB_INTERNAL
#define BLE_OTA_STORAGE_LIB_INTERNAL_ESP
#endif