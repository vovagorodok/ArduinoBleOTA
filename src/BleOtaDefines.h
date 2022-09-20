#pragma once

#if defined(ESP32)
#ifndef NIM_BLE_ARDUINO_LIB
#define NIM_BLE_ARDUINO_LIB
#endif
#else
#ifndef ARDUINO_BLE_LIB
#define ARDUINO_BLE_LIB
#define BLE_PULL_REQUIRED
#endif
#endif