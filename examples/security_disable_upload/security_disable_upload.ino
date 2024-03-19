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
#ifdef USE_ARDUINO_BLE_LIB
  BLE.poll();
#endif
  ArduinoBleOTA.pull();
}