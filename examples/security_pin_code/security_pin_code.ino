#include <ArduinoBleOTA.h>
#include <BleOtaSecurityOnConnect.h>

#ifdef BLE_OTA_LIB_NIM_BLE_ARDUINO
BleOtaSecurityOnConnect security;
#endif

void setup() {
  ArduinoBleOTA.begin("ArduinoBleOTA", InternalStorage);

#ifdef BLE_OTA_LIB_NIM_BLE_ARDUINO
  ArduinoBleOTA.setSecurityCallbacks(security);
  security.begin();
#endif
}

void loop() {
#ifdef BLE_OTA_LIB_ARDUINO_BLE
  BLE.poll();
#endif
  ArduinoBleOTA.pull();
}