#pragma once
#include "OTAStorage.h"
#include <NimBLEDevice.h>

class ArduinoBleOTAClass: public BLECharacteristicCallbacks
{
public:
    void begin(const std::string &deviceName, OTAStorage& storage);
    void begin(NimBLEServer* server, OTAStorage& storage);
    void begin(NimBLEServer* server, OTAStorage& storage,
               const char* serviceUUID, const char* rxChUUID, const char* txChUUID);
    void onWrite(BLECharacteristic* characteristic) override;

private:
    void onStartUpdate(BLECharacteristic* characteristic);
    void onUpdate(BLECharacteristic* characteristic);
    void send(const std::string& str);
    BLECharacteristic* txCharacteristic;
    OTAStorage* storage;
    long contentUploaded;
    long contentLength = 0;
};

static ArduinoBleOTAClass ArduinoBleOTA;