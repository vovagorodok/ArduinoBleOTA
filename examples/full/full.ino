#include <ArduinoBleOTA.h>
#include <BleOtaSecurityOnConnect.h>

#define DEVICE_NAME "ArduinoBleOTA"

#if defined(ARDUINO_ARCH_ESP32)
  #define HW_NAME "Example ESP32"
#elif defined(ARDUINO_ARCH_SAMD)
  #define HW_NAME "Example SAMD"
#else
  #define HW_NAME "Example HW"
#endif

BleOtaInfo info {
  "Example MF",
  HW_NAME,
  "Example SW",
  {1, 0, 0},
  {1, 0, 0}
};

#ifdef BLE_OTA_LOGS
#ifndef ARDUINO_ARCH_ESP32
extern "C" {
int _write(int fd, char *ptr, int len) {
  (void) fd;
  return Serial.write(ptr, len);
}
}
#endif
#endif

#ifdef BLE_OTA_LIB_NIM_BLE_ARDUINO
BleOtaSecurityOnConnect security;
#endif

void setup() {
#ifdef BLE_OTA_LOGS
  Serial.begin(115200);
  while (!Serial);
#endif

  ArduinoBleOTA.begin(DEVICE_NAME, InternalStorage, info);

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