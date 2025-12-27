#pragma once
#include "BleOtaDefines.h"
#if defined(BLE_OTA_BLE_LIB_ARDUINO_BLE)
#include "BleOtaLibArduinoBle.h"
#elif defined(BLE_OTA_BLE_LIB_NIM_BLE_ARDUINO)
#include "BleOtaLibNimBleArduino.h"
#elif defined(BLE_OTA_BLE_LIB_ESP32)
#include "BleOtaLibESP32.h"
#elif defined(BLE_OTA_BLE_LIB_FAKE)
#include "BleOtaLibFake.h"
#else
#error "Unsupported BLE library."
#endif