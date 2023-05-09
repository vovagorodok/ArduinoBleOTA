#include <ArduinoBleOTA.h>
#include <BleOtaSecurityOnConnect.h>

#ifdef USE_NIM_BLE_ARDUINO_LIB
BleOtaSecurityOnConnect security;
#endif

void setup() {
  ArduinoBleOTA.begin("ArduinoBleOTA", InternalStorage);

#ifdef USE_NIM_BLE_ARDUINO_LIB
  ArduinoBleOTA.setSecurityCallbacks(security);
  security.begin();
#endif
}

void loop() {
#ifndef USE_NIM_BLE_ARDUINO_LIB
  BLE.poll();
#endif
  ArduinoBleOTA.pull();
}