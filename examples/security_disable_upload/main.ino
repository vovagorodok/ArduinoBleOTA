#include <ArduinoBleOTA.h>

void setup() {
  ArduinoBleOTA.begin("ArduinoBleOTA", InternalStorage);

  for (int i = 0; i < 5000; i++) {
    loop();
    delay(1);
  }
  
  ArduinoBleOTA.disableUpload();
}

void loop() {
#ifndef USE_NIM_BLE_ARDUINO_LIB
  BLE.poll();
#endif
  ArduinoBleOTA.pull();
}