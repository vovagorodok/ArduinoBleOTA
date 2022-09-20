#pragma once

// Copied from https://github.com/JAndrassy/ArduinoOTA/blob/master/src/ArduinoOTA.h
#ifdef __AVR__
#if FLASHEND >= 0xFFFF
#include "InternalStorageAVR.h"
#endif
#elif defined(STM32F1xx)
#include <InternalStorageSTM32.h>
#elif defined(ARDUINO_ARCH_RP2040)
#include <InternalStorageRP2.h>
#elif defined(ESP8266) || defined(ESP32)
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