#include <ArduinoBleOTA.h>
#include <BleOtaSecurityOnConnect.h>

#define DEVICE_NAME "ArduinoBleOTA"

#if defined(ARDUINO_ARCH_ESP32)
  #define HW_NAME "Example ESP32"
#elif defined(ARDUINO_ARCH_ATMELSAM)
  #define HW_NAME "Example ATMELSAM"
#else
  #define HW_NAME "Example HW"
#endif

#define HW_VER {1, 0, 0}
#define SW_NAME "Example SW"
#define SW_VER {1, 0, 0}

#ifdef USE_NIM_BLE_ARDUINO_LIB
BleOtaSecurityOnConnect security;
#endif

void setup() {
  ArduinoBleOTA.begin(DEVICE_NAME, InternalStorage, HW_NAME, HW_VER, SW_NAME, SW_VER);

#ifdef USE_NIM_BLE_ARDUINO_LIB
  ArduinoBleOTA.setSecurityCallbacks(security);
  security.begin();
#endif
}

void loop() {
#ifdef USE_ARDUINO_BLE_LIB
  BLE.poll();
#endif
  ArduinoBleOTA.pull();
}