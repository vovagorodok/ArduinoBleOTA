#include <ArduinoBleOTA.h>

void setup() {
  ArduinoBleOTA.begin("ArduinoBleOTA", InternalStorage);

  for (int i = 0; i < 5000; i++) {
    loop();
    delay(1);
  }
  
  ArduinoBleOTA.setEnableUpload(false);
}

void loop() {
#ifdef BLE_OTA_LIB_ARDUINO_BLE
  BLE.poll();
#endif
  ArduinoBleOTA.pull();
}