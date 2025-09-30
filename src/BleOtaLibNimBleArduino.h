#pragma once
#include "BleOtaDefines.h"
#ifdef BLE_OTA_BLE_LIB_NIM_BLE_ARDUINO
#include "BleOtaUuids.h"
#include "BleOtaStorage.h"
#include "BleOtaInfo.h"
#include "BleOtaUploader.h"

class BleOtaLib: public BLECharacteristicCallbacks
{
public:
    BleOtaLib();

    bool begin(const std::string& deviceName,
               OTAStorage& storage,
               const BleOtaInfo& info = {},
               bool uploadEnable = true);
    bool begin(BLEServer* server,
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
    void begin(BLEService& service, const BleOtaInfo& info);
    void onWrite(BLECharacteristic* characteristic) override;
    void send(const uint8_t* data, size_t size);

    BLECharacteristic* _txCharacteristic;
    BleOtaUploader _uploader;
};

extern BleOtaLib ArduinoBleOTA;
#endif