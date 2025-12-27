#include "BleOtaLibFake.h"
#ifdef BLE_OTA_BLE_LIB_FAKE

BleOtaLib::BleOtaLib()
{}

bool BleOtaLib::begin(const char* deviceName,
                      OTAStorage& storage,
                      const BleOtaInfo& info,
                      bool uploadEnable)
{
    return false;
}

void BleOtaLib::begin(OTAStorage& storage,
                      const BleOtaInfo& info,
                      bool uploadEnable)
{}

bool BleOtaLib::begin(const std::string& deviceName,
                      OTAStorage& storage,
                      const BleOtaInfo& info,
                      bool uploadEnable)
{
    return false;
}

bool BleOtaLib::begin(BleOtaServerFake* server,
                      OTAStorage& storage,
                      const BleOtaInfo& info,
                      bool uploadEnable)
{
    return false;
}

void BleOtaLib::pull()
{}

void BleOtaLib::setUploadEnable(bool enable)
{}

bool BleOtaLib::setSignatureKey(const char* key, size_t size)
{
    return false;
}

void BleOtaLib::setPinCallbacks(BleOtaPinCallbacks& cb)
{}

void BleOtaLib::setUploadCallbacks(BleOtaUploadCallbacks& cb)
{}

void BleOtaLib::send(const uint8_t* data, size_t size)
{}

BleOtaLib ArduinoBleOTA{};
#endif