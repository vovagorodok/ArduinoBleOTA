#include <ArduinoBleOTA.h>
#include <BleOtaMultiservice.h>

// max name size with 128 bit uuid is 11
#define NAME "MultiSrv"
#define MY_SECOND_SERVICE_UUID "<any uuid>"

void setup() {
  initBle(NAME);

#ifndef USE_NIM_BLE_ARDUINO_LIB
  static BLEService service(MY_SECOND_SERVICE_UUID);
  BLE.addService(service);
#else
  auto* server = BLEDevice::createServer();
  auto* service = server->createService(MY_SECOND_SERVICE_UUID);
#endif

  ArduinoBleOTA.begin(InternalStorage);
  advertiseBle(NAME, MY_SECOND_SERVICE_UUID);
}

void loop() {
#ifndef USE_NIM_BLE_ARDUINO_LIB
  BLE.poll();
#endif
  ArduinoBleOTA.pull();
}