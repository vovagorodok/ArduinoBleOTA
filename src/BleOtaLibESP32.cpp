#include "BleOtaLibESP32.h"
#ifdef BLE_OTA_BLE_LIB_ESP32

BleOtaLib::BleOtaLib():
    _txCharacteristic(),
    _uploader()
{}

void BleOtaLib::begin(const std::string& deviceName,
                      OTAStorage& storage,
                      const BleOtaInfo& info,
                      bool uploadEnable)
{
    BLEDevice::init(deviceName);
    auto* server = BLEDevice::createServer();

    begin(server, storage, info, uploadEnable);

    auto* advertising = server->getAdvertising();
    advertising->addServiceUUID(BLE_OTA_SERVICE_UUID);
    advertising->setScanResponse(true);
    advertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    advertising->setMaxPreferred(0x12);
    advertising->start();
}

void BleOtaLib::begin(BLEServer* server,
                      OTAStorage& storage,
                      const BleOtaInfo& info,
                      bool uploadEnable)
{
    BLEDevice::setMTU(BLE_OTA_MTU_SIZE);

    _uploader.begin(storage, uploadEnable);
    auto* service = server->createService(BLE_OTA_SERVICE_UUID);

    auto* rxCharacteristic = service->createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_RX,
        BLECharacteristic::PROPERTY_WRITE_NR
    );
    rxCharacteristic->setCallbacks(this);

    auto* txCharacteristic = service->createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_TX,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    _txCharacteristic = txCharacteristic;

    begin(*service, info);

    auto* advertising = server->getAdvertising();
    service->start();
}

void BleOtaLib::begin(BLEService& service,
                      const BleOtaInfo& info)
{
    auto* mfNameCharacteristic = service.createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_MF_NAME,
        BLECharacteristic::PROPERTY_READ
    );
    mfNameCharacteristic->setValue(info.mfName);
    auto* hwNameCharacteristic = service.createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_HW_NAME,
        BLECharacteristic::PROPERTY_READ
    );
    hwNameCharacteristic->setValue(info.hwName);
    auto* swNameCharacteristic = service.createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_SW_NAME,
        BLECharacteristic::PROPERTY_READ
    );
    swNameCharacteristic->setValue(info.swName);
    auto* hwVerCharacteristic = service.createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_HW_VER,
        BLECharacteristic::PROPERTY_READ
    );
    hwVerCharacteristic->setValue((uint8_t*)&info.hwVersion, sizeof(BleOtaVersion));
    auto* swVerCharacteristic = service.createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_SW_VER,
        BLECharacteristic::PROPERTY_READ
    );
    swVerCharacteristic->setValue((uint8_t*)&info.swVersion, sizeof(BleOtaVersion));
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

void BleOtaLib::onWrite(BLECharacteristic* characteristic)
{
    auto value = characteristic->getValue();
    auto data = value.data();
    auto size = value.length();

    _uploader.handleData((uint8_t*)data, size);
}

void BleOtaLib::send(const uint8_t* data, size_t size)
{
    _txCharacteristic->setValue((uint8_t*)data, size);
    _txCharacteristic->notify();
}

BleOtaLib ArduinoBleOTA{};
#endif