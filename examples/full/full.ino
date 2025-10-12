#include <ArduinoBleOTA.h>
#include <BleOtaSecurityServer.h>

#define DEVICE_NAME "ArduinoBleOTA"

#define MF_NAME "Example MF"
#if defined(ARDUINO_ARCH_ESP32)
  #define HW_NAME "Example ESP32"
#elif defined(ARDUINO_ARCH_SAMD)
  #define HW_NAME "Example SAMD"
#else
  #define HW_NAME "Example HW"
#endif
#define SW_NAME "Example SW"
#define HW_VER {1, 0, 0}
#define SW_VER {1, 0, 0}

BleOtaInfo info {
  MF_NAME,
  HW_NAME,
  SW_NAME,
  HW_VER,
  SW_VER
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
BleOtaSecurityServer security;
#endif

void setup() {
#ifdef BLE_OTA_LOGS
  Serial.begin(115200);
  while (!Serial);
#endif

  ArduinoBleOTA.begin(DEVICE_NAME, InternalStorage, info);

#ifdef BLE_OTA_LIB_NIM_BLE_ARDUINO
  ArduinoBleOTA.setPinCallbacks(security);
  security.begin();
#endif
}

void loop() {
#ifdef BLE_OTA_LIB_ARDUINO_BLE
  BLE.poll();
#endif
  ArduinoBleOTA.pull();
}