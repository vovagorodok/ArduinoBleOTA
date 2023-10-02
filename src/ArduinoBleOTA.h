#pragma once

#include "BleOtaDefines.h"
#ifdef USE_NIM_BLE_ARDUINO_LIB
#include "ArduinoBleOtaClassNimBle.h"
#elif USE_NATIVE_ESP32_LIB
"ArduinoBleOtaClassESP32Ble.h"
#else
#include "ArduinoBleOtaClass.h"
#endif