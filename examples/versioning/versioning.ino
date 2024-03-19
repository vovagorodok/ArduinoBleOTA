#include <ArduinoBleOTA.h>

#define DEVICE_NAME "ArduinoBleOTA"
#define HW_NAME "Example HW"
#define HW_VER {1, 0, 0}
#define SW_NAME "Example SW"
#define SW_VER {1, 0, 0}

void setup() {
  ArduinoBleOTA.begin(DEVICE_NAME, InternalStorage, HW_NAME, HW_VER, SW_NAME, SW_VER);
}

void loop() {
#ifdef USE_ARDUINO_BLE_LIB
  BLE.poll();
#endif
  ArduinoBleOTA.pull();
}