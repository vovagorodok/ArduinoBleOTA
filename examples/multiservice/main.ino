#include <ArduinoBleOTA.h>
#include <BleOtaMultiservice.h>

// max name size with 128 bit uuid is 11
#define NAME "MultiSrv"
#define MY_SECOND_SERVICE_UUID "<any uuid>"

#ifndef USE_NIM_BLE_ARDUINO_LIB

BLEService service(MY_SECOND_SERVICE_UUID);

void setup() {
  initBle(NAME);

  BLE.addService(service);

  ArduinoBleOTA.begin(InternalStorage);
  advertiseBle(NAME, MY_SECOND_SERVICE_UUID);
}

void loop() {
  BLE.poll();
  ArduinoBleOTA.pull();
}

#else

void setup() {
  initBle(NAME);

  auto* server = BLEDevice::createServer();
  auto* service = server->createService(MY_SECOND_SERVICE_UUID);

  ArduinoBleOTA.begin(InternalStorage);
  advertiseBle(NAME, MY_SECOND_SERVICE_UUID);
}

void loop() {
  ArduinoBleOTA.pull();
}

#endif