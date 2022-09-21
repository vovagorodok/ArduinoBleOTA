#include "BleOtaDefines.h"
#if defined(NIM_BLE_ARDUINO_LIB)
#include "ArduinoBleOtaNimBle.h"
#include "BleOtaUploader.h"
#include "BleOtaUuids.h"
#include "BleOtaUtils.h"
#include "BleOtaSizes.h"

namespace
{
constexpr auto UNKNOWN = "UNKNOWN";
}

bool ArduinoBleOTAClass::begin(const std::string& deviceName, OTAStorage& storage)
{
    return begin(deviceName, storage, UNKNOWN, {}, UNKNOWN, {});
}

bool ArduinoBleOTAClass::begin(OTAStorage& storage)
{
    return begin(storage, UNKNOWN, {}, UNKNOWN, {});
}

bool ArduinoBleOTAClass::begin(const std::string& deviceName, OTAStorage& storage,
                               const std::string& hwName, BleOtaVersion hwVersion,
                               const std::string& swName, BleOtaVersion swVersion)
{
    BLEDevice::init(deviceName);
    auto* server = BLEDevice::createServer();

    if(!begin(storage, hwName, hwVersion, swName, swVersion))
        return false;

    auto* advertising = server->getAdvertising();
    advertising->setScanResponse(true);
    advertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    advertising->setMaxPreferred(0x12);
    return advertising->start();
}

bool ArduinoBleOTAClass::begin(OTAStorage& storage,
                               const std::string& hwName, BleOtaVersion hwVersion,
                               const std::string& swName, BleOtaVersion swVersion)
{
    auto* server = BLEDevice::createServer();
    BLEDevice::setMTU(BLE_OTA_MTU_SIZE);

    bleOtaUploader.begin(storage);
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

    begin(*service, hwName, hwVersion, swName, swVersion);

    auto* advertising = server->getAdvertising();
    advertising->addServiceUUID(BLE_OTA_SERVICE_UUID);
    return service->start();
}

void ArduinoBleOTAClass::begin(BLEService& service,
                               const std::string& hwName, BleOtaVersion hwVersion,
                               const std::string& swName, BleOtaVersion swVersion)
{
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
    hwVerCharacteristic->setValue(refToAddr(hwVersion), sizeof(BleOtaVersion));
    auto* swVerCharacteristic = service.createCharacteristic(
        BLE_OTA_CHARACTERISTIC_UUID_SW_VER,
        NIMBLE_PROPERTY::READ
    );
    swVerCharacteristic->setValue(refToAddr(swVersion), sizeof(BleOtaVersion));
}

void ArduinoBleOTAClass::pull()
{
    bleOtaUploader.pull();
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