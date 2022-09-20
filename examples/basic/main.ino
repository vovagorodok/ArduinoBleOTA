#include <ArduinoBleOTA.h>

void setup() {
  ArduinoBleOTA.begin("ArduinoBleOTA", InternalStorage);
}

void loop() {
#if defined(BLE_PULL_REQUIRED)
  BLE.poll();
#endif
  ArduinoBleOTA.pull();
}