#pragma once
#include "BleOtaDefines.h"
#ifdef BLE_OTA_BLE_LIB_ARDUINO_BLE
#include "BleOtaUuids.h"
#include "BleOtaStorage.h"
#include "BleOtaInfo.h"
#include "BleOtaUploader.h"

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
    void pull();

    void setUploadEnable(bool enable);
    bool setSignatureKey(const char* key, size_t size);
    void setPinCallbacks(BleOtaPinCallbacks&);
    void setUploadCallbacks(BleOtaUploadCallbacks&);

private:
    friend BleOtaUploader;
    void begin(const BleOtaInfo& info);
    void onWrite(const BLECharacteristic& characteristic);
    void send(const uint8_t* data, size_t size);
    static void onWrite(BLEDevice central, BLECharacteristic characteristic);

    BleOtaUploader _uploader;
};

extern BleOtaLib ArduinoBleOTA;
#endif