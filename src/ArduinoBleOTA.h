#pragma once
#include "OTAStorage.h"
#include <NimBLEDevice.h>
#include <CRC32.h>

class ArduinoBleOTAClass: public BLECharacteristicCallbacks
{
public:
    bool begin(const std::string &deviceName, OTAStorage& storage);
    bool begin(OTAStorage& storage);
    void update();

private:
    void onWrite(BLECharacteristic* characteristic) override;
    void handleBegin(const uint8_t* data, size_t length);
    void handlePackage(const uint8_t* data, size_t length);
    void handleEnd(const uint8_t* data, size_t length);
    void handleInstall();
    void send(uint8_t head);
    void stopUpload();
    CRC32 crc;
    BLECharacteristic* txCharacteristic;
    OTAStorage* storage;
    uint32_t currentLength;
    uint32_t firmwareLength = 0;
    bool updating = false;
    bool installing = false;
};

static ArduinoBleOTAClass ArduinoBleOTA;