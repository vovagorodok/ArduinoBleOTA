#include <ArduinoBleOTA.h>
#include <BleOtaSecurityOnConnect.h>

#define DEVICE_NAME "ArduinoBleOTA"

#ifdef ESP32
  #define HW_NAME "Example Hardware ESP32"
#else
  #define HW_NAME "Example Hardware"
#endif

#define HW_VER {1, 0, 0}
#define SW_NAME "Example Software"
#define SW_VER {1, 0, 0}

#ifdef USE_NIM_BLE_ARDUINO_LIB
BleOtaSecurityOnConnect security;
#endif

void setup() {
  ArduinoBleOTA.begin(DEVICE_NAME, InternalStorage, HW_NAME, HW_VER, SW_NAME, SW_VER);

#ifdef USE_NIM_BLE_ARDUINO_LIB
  ArduinoBleOTA.setSecurity(security);
  security.begin();
#endif
}

void loop() {
#ifndef USE_NIM_BLE_ARDUINO_LIB
  BLE.poll();
#endif
  ArduinoBleOTA.pull();
}