#include "BleOtaDefines.h"
#ifdef USE_NATIVE_ESP32_BLE_LIB
#include "ArduinoBleOtaClassNativeESP32.h"
#include "BleOtaUploader.h"
#include "BleOtaUuids.h"
#include "BleOtaSizes.h"

namespace
{
static BleOtaSecurityCallbacks dummySecurityCallbacks{};
static BleOtaUploadCallbacks dummyUploadCallbacks{};
}

ArduinoBleOTAClass::ArduinoBleOTAClass() :
    txCharacteristic(),
    securityCallbacks(&dummySecurityCallbacks),
    uploadCallbacks(&dummyUploadCallbacks)
{}

bool ArduinoBleOTAClass::begin(const std::string& deviceName, OTAStorage& storage,
                               const std::string& hwName, BleOtaVersion hwVersion,
                               const std::string& swName, BleOtaVersion swVersion,
                               bool enableUpload)
{
    BLEDevice::init(deviceName);
    auto* server = BLEDevice::createServer();

    if(!begin(storage, hwName, hwVersion, swName, swVersion, enableUpload))
        return false;

    auto* advertising = server->getAdvertising();
    advertising->setScanResponse(true);
    advertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    advertising->setMaxPreferred(0x12);
    advertising->start();
    return true;
}

bool ArduinoBleOTAClass::begin(OTAStorage& storage,
                               const std::string& hwName, BleOtaVersion hwVersion,
                               const std::string& swName, BleOtaVersion swVersion,
                               bool enableUpload)
{
    auto* server = BLEDevice::createServer();
    BLEDevice::setMTU(BLE_OTA_MTU_SIZE);

    bleOtaUploader.begin(storage);
    bleOtaUploader.setEnabling(enableUpload);
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
    this->txCharacteristic = txCharacteristic;

    begin(*service, hwName, hwVersion, swName, swVersion);

    auto* advertising = server->getAdvertising();
    advertising->addServiceUUID(BLE_OTA_SERVICE_UUID);
    service->start();
    return true;
}

void ArduinoBleOTAClass::begin(BLEService& service,
                               const std::string& hwName, BleOtaVersion hwVersion,
                               const std::string& swName, BleOtaVersion swVersion)
{
    auto* hwNameCharacteristic = service.createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_HW_NAME,
        BLECharacteristic::PROPERTY_READ
    );
    hwNameCharacteristic->setValue(hwName);
    auto* swNameCharacteristic = service.createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_SW_NAME,
        BLECharacteristic::PROPERTY_READ
    );
    swNameCharacteristic->setValue(swName);
    auto* hwVerCharacteristic = service.createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_HW_VER,
        BLECharacteristic::PROPERTY_READ
    );
    hwVerCharacteristic->setValue((uint8_t*)(&hwVersion), sizeof(BleOtaVersion));
    auto* swVerCharacteristic = service.createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_SW_VER,
        BLECharacteristic::PROPERTY_READ
    );
    swVerCharacteristic->setValue((uint8_t*)(&swVersion), sizeof(BleOtaVersion));
}

void ArduinoBleOTAClass::pull()
{
    bleOtaUploader.pull();
}

void ArduinoBleOTAClass::enableUpload()
{
    bleOtaUploader.setEnabling(true);
}

void ArduinoBleOTAClass::disableUpload()
{
    bleOtaUploader.setEnabling(false);
}

void ArduinoBleOTAClass::setSecurityCallbacks(BleOtaSecurityCallbacks& cb)
{
    securityCallbacks = &cb;
}

void ArduinoBleOTAClass::setUploadCallbacks(BleOtaUploadCallbacks& cb)
{
    uploadCallbacks = &cb;
}

void ArduinoBleOTAClass::onWrite(BLECharacteristic* characteristic)
{
    auto value = characteristic->getValue();
    auto data = value.data();
    auto length = value.length();

    bleOtaUploader.onData((uint8_t*)data, length);
}

void ArduinoBleOTAClass::send(const uint8_t* data, size_t length)
{
    txCharacteristic->setValue((uint8_t*)data, length);
    txCharacteristic->notify();
}

ArduinoBleOTAClass ArduinoBleOTA{};
#endif