#include <ArduinoBleOTA.h>
#include <BleOtaMultiservice.h>

// max name size with 128 bit uuid is 11
#define NAME "MultiSrv"
#define MY_SECOND_SERVICE_UUID "<any uuid>"

void setup() {
  initBle(NAME);

#ifdef USE_ARDUINO_BLE_LIB
  static BLEService service(MY_SECOND_SERVICE_UUID);
  BLE.addService(service);
#else
  auto* server = BLEDevice::createServer();
  auto* service = server->createService(MY_SECOND_SERVICE_UUID);
  service->start();
#endif

  ArduinoBleOTA.begin(InternalStorage);
  advertiseBle(NAME, MY_SECOND_SERVICE_UUID);
}

void loop() {
#ifdef USE_ARDUINO_BLE_LIB
  BLE.poll();
#endif
  ArduinoBleOTA.pull();
}