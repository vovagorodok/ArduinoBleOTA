#include <ArduinoBleOTA.h>
#include <BleOtaMultiservice.h>

// max name size with 128 bit uuid is 11
#define NAME "MultiSrv"
#define MY_SECOND_SERVICE_UUID "<any uuid>"

void setup() {
  initBle(NAME);

  // add your ble services here

  ArduinoBleOTA.begin(InternalStorage);
  advertiseBle(NAME, MY_SECOND_SERVICE_UUID);
}

void loop() {
#if defined(BLE_PULL_REQUIRED)
  BLE.poll();
#endif
  ArduinoBleOTA.pull();
}