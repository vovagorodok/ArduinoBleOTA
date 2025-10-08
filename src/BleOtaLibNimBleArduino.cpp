#include "BleOtaLibNimBleArduino.h"
#ifdef BLE_OTA_BLE_LIB_NIM_BLE_ARDUINO

BleOtaLib::BleOtaLib():
    _txCharacteristic(),
    _uploader()
{}

bool BleOtaLib::begin(const std::string& deviceName,
                      OTAStorage& storage,
                      const BleOtaInfo& info,
                      bool uploadEnable)
{
    BLEDevice::init(deviceName);
    auto* server = BLEDevice::createServer();

    if(!begin(server, storage, info, uploadEnable))
        return false;

    auto* advertising = server->getAdvertising();
    advertising->addServiceUUID(BLE_OTA_SERVICE_UUID);
#ifdef BLE_OTA_BLE_LIB_NIM_BLE_ARDUINO_V1
    advertising->setScanResponse(true);
    advertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    advertising->setMaxPreferred(0x12);
#endif
    return advertising->start();
}

bool BleOtaLib::begin(BLEServer* server,
                      OTAStorage& storage,
                      const BleOtaInfo& info,
                      bool uploadEnable)
{
    BLEDevice::setMTU(BLE_OTA_MTU_SIZE);

    _uploader.begin(storage, uploadEnable);
    auto* service = server->createService(BLE_OTA_SERVICE_UUID);

    auto* rxCharacteristic = service->createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_RX,
        NIMBLE_PROPERTY::WRITE_NR
    );
    rxCharacteristic->setCallbacks(this);

    auto* txCharacteristic = service->createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_TX,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    _txCharacteristic = txCharacteristic;

    begin(*service, info);

    return service->start();
}

void BleOtaLib::begin(BLEService& service, const BleOtaInfo& info)
{
    auto* mfNameCharacteristic = service.createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_MF_NAME,
        NIMBLE_PROPERTY::READ
    );
    mfNameCharacteristic->setValue(reinterpret_cast<const uint8_t*>(info.mfName), strlen(info.mfName));
    auto* hwNameCharacteristic = service.createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_HW_NAME,
        NIMBLE_PROPERTY::READ
    );
    hwNameCharacteristic->setValue(reinterpret_cast<const uint8_t*>(info.hwName), strlen(info.hwName));
    auto* swNameCharacteristic = service.createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_SW_NAME,
        NIMBLE_PROPERTY::READ
    );
    swNameCharacteristic->setValue(reinterpret_cast<const uint8_t*>(info.swName), strlen(info.swName));
    auto* hwVerCharacteristic = service.createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_HW_VER,
        NIMBLE_PROPERTY::READ
    );
    hwVerCharacteristic->setValue(reinterpret_cast<const uint8_t*>(&info.hwVersion), sizeof(BleOtaVersion));
    auto* swVerCharacteristic = service.createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_SW_VER,
        NIMBLE_PROPERTY::READ
    );
    swVerCharacteristic->setValue(reinterpret_cast<const uint8_t*>(&info.swVersion), sizeof(BleOtaVersion));
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

#ifdef BLE_OTA_BLE_LIB_NIM_BLE_ARDUINO_V1
void BleOtaLib::onWrite(BLECharacteristic* characteristic)
#else
void BleOtaLib::onWrite(BLECharacteristic* characteristic, BLEConnInfo& connInfo)
#endif
{
    auto value = characteristic->getValue();
    auto data = value.data();
    auto size = value.length();

    _uploader.handleData(data, size);
}

void BleOtaLib::send(const uint8_t* data, size_t size)
{
    _txCharacteristic->setValue(data, size);
    _txCharacteristic->notify();
}

BleOtaLib ArduinoBleOTA{};
#endif