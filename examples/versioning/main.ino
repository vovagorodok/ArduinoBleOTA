#include <ArduinoBleOTA.h>

#define DEVICE_NAME "ArduinoBleOTA"
#if defined(ESP32)
#define HW_NAME "Example Hardware"
#else
#define HW_NAME "Hardware"
#endif
#define HW_VER {1, 0, 0}
#define SW_NAME "Example Software"
#define SW_VER {1, 0, 0}

void setup() {
  ArduinoBleOTA.begin(DEVICE_NAME, InternalStorage, HW_NAME, HW_VER, SW_NAME, SW_VER);
}

void loop() {
#if defined(BLE_PULL_REQUIRED)
  BLE.poll();
#endif
  ArduinoBleOTA.pull();
}