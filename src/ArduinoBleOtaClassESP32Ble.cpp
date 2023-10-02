#include "BleOtaDefines.h"
#ifdef USE_NATIVE_ESP32_LIB 
#include "ArduinoBleOtaClassESP32Ble.h"
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
                               bool enableUpload, bool advertise)
{
    BLEDevice::init(deviceName);
    auto* server = BLEDevice::createServer();

    begin(server, storage, hwName, hwVersion, swName, swVersion, enableUpload, advertise);

    if(advertise){
        auto* advertising = server->getAdvertising();
        advertising->setScanResponse(true);
        advertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
        advertising->setMaxPreferred(0x12);
        advertising->addServiceUUID(BLE_OTA_SERVICE_UUID);
        advertising->start();
    }
    return true;
}

bool ArduinoBleOTAClass::begin(OTAStorage &storage,
                               const std::string &hwName, BleOtaVersion hwVersion,
                               const std::string &swName, BleOtaVersion swVersion,
                               bool enableUpload, bool advertise)
{
    BLEServer * server =  BLEDevice::createServer();

    BLEService * service=begin(server, storage, hwName, hwVersion, swName, swVersion);

    return true;
}

BLEService *ArduinoBleOTAClass::begin(BLEServer *server, OTAStorage &storage,
                                      const std::string &hwName, BleOtaVersion hwVersion,
                                      const std::string &swName, BleOtaVersion swVersion,
                                      bool enableUpload, bool advertise)
{
    BLEDevice::setMTU(BLE_OTA_MTU_SIZE);

    bleOtaUploader.begin(storage);
    bleOtaUploader.setEnabling(enableUpload);
    BLEService * service = server->createService(BLE_OTA_SERVICE_UUID);

    auto *rxCharacteristic = service->createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_RX,
        NIMBLE_PROPERTY::PROPERTY_WRITE_NR);
    rxCharacteristic->setCallbacks(this);

    auto *txCharacteristic = service->createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_TX,
        NIMBLE_PROPERTY::PROPERTY_READ | NIMBLE_PROPERTY::PROPERTY_NOTIFY);
    this->txCharacteristic = txCharacteristic;

    begin(*service, hwName, hwVersion, swName, swVersion);
    if(advertise){
        auto *advertising = server->getAdvertising();
        advertising->setScanResponse(true);
        advertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
        advertising->setMaxPreferred(0x12);
        advertising->addServiceUUID(BLE_OTA_SERVICE_UUID);
        service->start();
    }
    return service;
}

void ArduinoBleOTAClass::begin(BLEService& service,
                               const std::string& hwName, BleOtaVersion hwVersion,
                               const std::string& swName, BleOtaVersion swVersion)
{
    auto* hwNameCharacteristic = service.createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_HW_NAME,
        NIMBLE_PROPERTY::PROPERTY_READ
    );
    hwNameCharacteristic->setValue(hwName);
    auto* swNameCharacteristic = service.createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_SW_NAME,
        NIMBLE_PROPERTY::PROPERTY_READ
    );
    swNameCharacteristic->setValue(swName);
    auto* hwVerCharacteristic = service.createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_HW_VER,
        NIMBLE_PROPERTY::PROPERTY_READ
    );
    hwVerCharacteristic->setValue((uint8_t*)(&hwVersion), sizeof(BleOtaVersion));
    auto* swVerCharacteristic = service.createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_SW_VER,
        NIMBLE_PROPERTY::PROPERTY_READ
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

void ArduinoBleOTAClass::setSecurityCallbacks(BleOtaSecurityCallbacks &cb)
{
    securityCallbacks = &cb;
}

void ArduinoBleOTAClass::setUploadCallbacks(BleOtaUploadCallbacks &cb)
{
    uploadCallbacks = &cb;
}

void ArduinoBleOTAClass::onWrite(BLECharacteristic* characteristic)
{
    auto value = characteristic->getValue();
    auto data = value.data();
    auto length = value.length();

    bleOtaUploader.onData((uint8_t *) data, length);
}

void ArduinoBleOTAClass::send(const uint8_t* data, size_t length)
{
    txCharacteristic->setValue((uint8_t*)data, length);
    txCharacteristic->notify();
}

ArduinoBleOTAClass ArduinoBleOTA{};
#endif
