#include <ArduinoBleOTA.h>

void setup() {
  ArduinoBleOTA.begin("ArduinoBleOTA", InternalStorage);
}

void loop() {
#ifndef USE_NIM_BLE_ARDUINO_LIB
  BLE.poll();
#endif
  ArduinoBleOTA.pull();
}