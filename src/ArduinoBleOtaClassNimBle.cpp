#include "BleOtaDefines.h"
#ifdef USE_NIM_BLE_ARDUINO_LIB
#include "ArduinoBleOtaClassNimBle.h"
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
                               const std::string& mfName,
                               const std::string& hwName, BleOtaVersion hwVersion,
                               const std::string& swName, BleOtaVersion swVersion,
                               bool enableUpload)
{
    BLEDevice::init(deviceName);
    auto* server = BLEDevice::createServer();

    if(!begin(storage, mfName, hwName, hwVersion, swName, swVersion, enableUpload))
        return false;

    auto* advertising = server->getAdvertising();
    advertising->setScanResponse(true);
    advertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    advertising->setMaxPreferred(0x12);
    return advertising->start();
}

bool ArduinoBleOTAClass::begin(OTAStorage& storage,
                               const std::string& mfName,
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
        NIMBLE_PROPERTY::WRITE_NR
    );
    rxCharacteristic->setCallbacks(this);

    auto* txCharacteristic = service->createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_TX,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    this->txCharacteristic = txCharacteristic;

    begin(*service, mfName, hwName, hwVersion, swName, swVersion);

    auto* advertising = server->getAdvertising();
    advertising->addServiceUUID(BLE_OTA_SERVICE_UUID);
    return service->start();
}

void ArduinoBleOTAClass::begin(BLEService& service,
                               const std::string& mfName,
                               const std::string& hwName, BleOtaVersion hwVersion,
                               const std::string& swName, BleOtaVersion swVersion)
{
    auto* mfNameCharacteristic = service.createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_MF_NAME,
        NIMBLE_PROPERTY::READ
    );
    mfNameCharacteristic->setValue(mfName);
    auto* hwNameCharacteristic = service.createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_HW_NAME,
        NIMBLE_PROPERTY::READ
    );
    hwNameCharacteristic->setValue(hwName);
    auto* swNameCharacteristic = service.createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_SW_NAME,
        NIMBLE_PROPERTY::READ
    );
    swNameCharacteristic->setValue(swName);
    auto* hwVerCharacteristic = service.createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_HW_VER,
        NIMBLE_PROPERTY::READ
    );
    hwVerCharacteristic->setValue((const uint8_t*)(&hwVersion), sizeof(BleOtaVersion));
    auto* swVerCharacteristic = service.createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_SW_VER,
        NIMBLE_PROPERTY::READ
    );
    swVerCharacteristic->setValue((const uint8_t*)(&swVersion), sizeof(BleOtaVersion));
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

    bleOtaUploader.onData(data, length);
}

void ArduinoBleOTAClass::send(const uint8_t* data, size_t length)
{
    txCharacteristic->setValue(data, length);
    txCharacteristic->notify();
}

ArduinoBleOTAClass ArduinoBleOTA{};
#endif