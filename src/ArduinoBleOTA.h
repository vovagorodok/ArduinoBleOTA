#pragma once
#include "OTAStorage.h"
#include <NimBLEDevice.h>
#include <CRC32.h>

struct Version
{
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
};

class ArduinoBleOTAClass: public BLECharacteristicCallbacks
{
public:
    bool begin(const std::string &deviceName, OTAStorage& storage);
    bool begin(OTAStorage& storage);
    bool begin(const std::string &deviceName, OTAStorage& storage,
               const std::string &hwName, Version hwVersion,
               const std::string &swName, Version swVersion);
    bool begin(OTAStorage& storage,
               const std::string &hwName, Version hwVersion,
               const std::string &swName, Version swVersion);
    void pull();

private:
    void begin(BLEService& service,
               const std::string &hwName, Version hwVersion,
               const std::string &swName, Version swVersion);
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
    bool uploading = false;
    bool installing = false;
};

static ArduinoBleOTAClass ArduinoBleOTA;