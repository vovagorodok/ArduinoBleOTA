#include <ArduinoBleOTA.h>

#define DEVICE_NAME "ArduinoBleOTA"
BleOtaInfo info {
  "Example MF",
  "Example HW",
  "Example SW",
  {1, 0, 0},
  {1, 0, 0}
};

void setup() {
  ArduinoBleOTA.begin(DEVICE_NAME, InternalStorage, info);
}

void loop() {
#ifdef BLE_OTA_LIB_ARDUINO_BLE
  BLE.poll();
#endif
  ArduinoBleOTA.pull();
}