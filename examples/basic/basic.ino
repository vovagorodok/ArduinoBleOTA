#include <ArduinoBleOTA.h>

void setup() {
  ArduinoBleOTA.begin("ArduinoBleOTA", InternalStorage);
}

void loop() {
#ifdef USE_ARDUINO_BLE_LIB
  BLE.poll();
#endif
  ArduinoBleOTA.pull();
}