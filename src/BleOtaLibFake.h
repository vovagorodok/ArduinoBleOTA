#pragma once
#include "BleOtaDefines.h"
#ifdef BLE_OTA_BLE_LIB_FAKE
#include "BleOtaUuids.h"
#include "BleOtaStorage.h"
#include "BleOtaInfo.h"
#include "BleOtaUploader.h"
#include <string>

using BleOtaServerFake = int;

class BleOtaLib
{
public:
    BleOtaLib();

    bool begin(const char* deviceName,
               OTAStorage& storage,
               const BleOtaInfo& info = {},
               bool uploadEnable = true);
    void begin(OTAStorage& storage,
               const BleOtaInfo& info = {},
               bool uploadEnable = true);
    bool begin(const std::string& deviceName,
               OTAStorage& storage,
               const BleOtaInfo& info = {},
               bool uploadEnable = true);
    bool begin(BleOtaServerFake* server,
               OTAStorage& storage,
               const BleOtaInfo& info = {},
               bool uploadEnable = true);
    void pull();

    void setUploadEnable(bool enable);
    bool setSignatureKey(const char* key, size_t size);
    void setPinCallbacks(BleOtaPinCallbacks&);
    void setUploadCallbacks(BleOtaUploadCallbacks&);

private:
    friend BleOtaUploader;
    void send(const uint8_t* data, size_t size);
};

extern BleOtaLib ArduinoBleOTA;
#endif