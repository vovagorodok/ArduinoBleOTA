#pragma once
#include "OTAStorage.h"
#include <NimBLEDevice.h>
#include <CRC32.h>

class ArduinoBleOTAClass: public BLECharacteristicCallbacks
{
public:
    bool begin(const std::string &deviceName, OTAStorage& storage);
    bool begin(NimBLEServer* server, OTAStorage& storage);
    bool begin(NimBLEServer* server, OTAStorage& storage,
               const char* serviceUUID, const char* rxChUUID, const char* txChUUID);
    void onWrite(BLECharacteristic* characteristic) override;

private:
    void beginUpdate(uint8_t* data, size_t length);
    void handlePackage(uint8_t* data, size_t length);
    void endUpdate(uint8_t* data, size_t length);
    void send(uint8_t head);
    void stopUpdate();
    CRC32 crc;
    BLECharacteristic* txCharacteristic;
    OTAStorage* storage;
    uint32_t currentLength;
    uint32_t firmwareLength = 0;
    bool updating = false;
};

static ArduinoBleOTAClass ArduinoBleOTA;