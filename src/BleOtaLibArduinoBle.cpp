#include "BleOtaLibArduinoBle.h"
#ifdef BLE_OTA_BLE_LIB_ARDUINO_BLE

namespace
{
BLEService service(BLE_OTA_SERVICE_UUID);
BLECharacteristic rxCharacteristic(BLE_OTA_CHARACTERISTIC_UUID_RX, BLEWriteWithoutResponse, BLE_OTA_MAX_ATTR_SIZE);
BLECharacteristic txCharacteristic(BLE_OTA_CHARACTERISTIC_UUID_TX, BLERead | BLENotify, BLE_OTA_MAX_ATTR_SIZE);
BLEStringCharacteristic mfNameCharacteristic(BLE_OTA_CHARACTERISTIC_UUID_MF_NAME, BLERead, BLE_OTA_MAX_ATTR_SIZE);
BLEStringCharacteristic hwNameCharacteristic(BLE_OTA_CHARACTERISTIC_UUID_HW_NAME, BLERead, BLE_OTA_MAX_ATTR_SIZE);
BLEStringCharacteristic swNameCharacteristic(BLE_OTA_CHARACTERISTIC_UUID_SW_NAME, BLERead, BLE_OTA_MAX_ATTR_SIZE);
BLECharacteristic hwVerCharacteristic(BLE_OTA_CHARACTERISTIC_UUID_HW_VER, BLERead, sizeof(BleOtaVersion), true);
BLECharacteristic swVerCharacteristic(BLE_OTA_CHARACTERISTIC_UUID_SW_VER, BLERead, sizeof(BleOtaVersion), true);
}

BleOtaLib::BleOtaLib():
    _uploader()
{}

bool BleOtaLib::begin(const char* deviceName,
                      OTAStorage& storage,
                      const BleOtaInfo& info,
                      bool uploadEnable)
{
    if (!BLE.begin())
        return false;

    BLE.setLocalName(deviceName);
    BLE.setDeviceName(deviceName);

    begin(storage, info, uploadEnable);

    return BLE.setAdvertisedService(service) and BLE.advertise();
}

void BleOtaLib::begin(OTAStorage& storage,
                      const BleOtaInfo& info,
                      bool uploadEnable)
{
    _uploader.begin(storage, uploadEnable);
    service.addCharacteristic(rxCharacteristic);
    service.addCharacteristic(txCharacteristic);
    rxCharacteristic.setEventHandler(BLEWritten, onWrite);

    begin(info);

    BLE.addService(service);
}

void BleOtaLib::begin(const BleOtaInfo& info)
{
    service.addCharacteristic(mfNameCharacteristic);
    mfNameCharacteristic.setValue(info.mfName);
    service.addCharacteristic(hwNameCharacteristic);
    hwNameCharacteristic.setValue(info.hwName);
    service.addCharacteristic(swNameCharacteristic);
    swNameCharacteristic.setValue(info.swName);
    service.addCharacteristic(hwVerCharacteristic);
    hwVerCharacteristic.setValue(reinterpret_cast<const uint8_t*>(&info.hwVersion), sizeof(BleOtaVersion));
    service.addCharacteristic(swVerCharacteristic);
    swVerCharacteristic.setValue(reinterpret_cast<const uint8_t*>(&info.swVersion), sizeof(BleOtaVersion));
}

void BleOtaLib::pull()
{
    _uploader.pull();
}

void BleOtaLib::setUploadEnable(bool enable)
{
    _uploader.setEnable(enable);
}

bool BleOtaLib::setSignatureKey(const char* key, size_t size)
{
    return _uploader.setSignatureKey(key, size);
}

void BleOtaLib::setPinCallbacks(BleOtaPinCallbacks& cb)
{
    _uploader.setPinCallbacks(cb);
}

void BleOtaLib::setUploadCallbacks(BleOtaUploadCallbacks& cb)
{
    _uploader.setUploadCallbacks(cb);
}

void BleOtaLib::onWrite(const BLECharacteristic& characteristic)
{
    _uploader.onData(rxCharacteristic.value(), rxCharacteristic.valueLength());
}

void BleOtaLib::send(const uint8_t* data, size_t size)
{
    txCharacteristic.setValue(data, size);
}

void BleOtaLib::onWrite(BLEDevice central, BLECharacteristic characteristic)
{
    ArduinoBleOTA.onWrite(characteristic);
}

BleOtaLib ArduinoBleOTA{};
#endif