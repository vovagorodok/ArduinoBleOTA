#include <ArduinoBleOTA.h>

void setup() {
  BLEDevice::init("ArduinoBleOTA");
  auto* server = BLEDevice::createServer();

  // add your ble services here

  ArduinoBleOTA.begin(InternalStorage);
  server->startAdvertising();
}

void loop() {
  ArduinoBleOTA.update();
}