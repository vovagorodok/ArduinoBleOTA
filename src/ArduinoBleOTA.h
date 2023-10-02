#pragma once

#ifdef USE_NIM_BLE_ARDUINO_LIB
#include "ArduinoBleOtaClassNimBle.h"
#elif USE_NATIVE_ESP32_LIB
"ArduinoBleOtaClassESP32Ble.h"
#else
#ifndef USE_ARDUINO_BLE_LIB
#define USE_ARDUINO_BLE_LIB
#endif
#include "ArduinoBleOtaClass.h"
#endif