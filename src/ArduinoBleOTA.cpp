#include "ArduinoBleOTA.h"
#include "BleOtaUploader.h"
#include "BleOtaUtils.h"
#include "BleOtaSizes.h"

namespace
{
#define OTA_SERVICE_UUID                "15c155ca-36c5-11ed-adc0-9741d6a72f04"
#define OTA_CHARACTERISTIC_UUID_RX      "15c1564c-36c5-11ed-adc1-a3d6cf5cc2a4"
#define OTA_CHARACTERISTIC_UUID_TX      "15c156e2-36c5-11ed-adc2-7396d4fd413a"
#define OTA_CHARACTERISTIC_UUID_HW_NAME "15c1576e-36c5-11ed-adc3-8799895de51e"
#define OTA_CHARACTERISTIC_UUID_HW_VER  "15c157fa-36c5-11ed-adc4-579c60267b47"
#define OTA_CHARACTERISTIC_UUID_SW_NAME "15c15886-36c5-11ed-adc5-1bc0d0a6069d"
#define OTA_CHARACTERISTIC_UUID_SW_VER  "15c1591c-36c5-11ed-adc6-dbe9603dbf19"

constexpr auto UNKNOWN = "UNKNOWN";
}

bool ArduinoBleOTAClass::begin(const std::string &deviceName, OTAStorage& storage)
{
    return begin(deviceName, storage, UNKNOWN, {}, UNKNOWN, {});
}

bool ArduinoBleOTAClass::begin(OTAStorage& storage)
{
    return begin(storage, UNKNOWN, {}, UNKNOWN, {});
}

bool ArduinoBleOTAClass::begin(const std::string &deviceName, OTAStorage& storage,
                               const std::string &hwName, Version hwVersion,
                               const std::string &swName, Version swVersion)
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
                               const std::string &hwName, Version hwVersion,
                               const std::string &swName, Version swVersion)
{
    auto* server = BLEDevice::createServer();
    BLEDevice::setMTU(BLE_OTA_MTU_SIZE);

    bleOtaUploader.begin(storage);
    auto* service = server->createService(OTA_SERVICE_UUID);

    auto* rxCharacteristic = service->createCharacteristic(
        OTA_CHARACTERISTIC_UUID_RX,
        NIMBLE_PROPERTY::WRITE_NR
    );
    rxCharacteristic->setCallbacks(this);

    auto* txCharacteristic = service->createCharacteristic(
        OTA_CHARACTERISTIC_UUID_TX,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    this->txCharacteristic = txCharacteristic;

    begin(*service, hwName, hwVersion, swName, swVersion);

    auto* advertising = server->getAdvertising();
    advertising->addServiceUUID(OTA_SERVICE_UUID);
    return service->start();
}

void ArduinoBleOTAClass::begin(BLEService& service,
                               const std::string &hwName, Version hwVersion,
                               const std::string &swName, Version swVersion)
{
    auto* hwNameCharacteristic = service.createCharacteristic(
        OTA_CHARACTERISTIC_UUID_HW_NAME,
        NIMBLE_PROPERTY::READ
    );
    hwNameCharacteristic->setValue(hwName);
    auto* swNameCharacteristic = service.createCharacteristic(
        OTA_CHARACTERISTIC_UUID_SW_NAME,
        NIMBLE_PROPERTY::READ
    );
    swNameCharacteristic->setValue(swName);
    auto* hwVerCharacteristic = service.createCharacteristic(
        OTA_CHARACTERISTIC_UUID_HW_VER,
        NIMBLE_PROPERTY::READ
    );
    hwVerCharacteristic->setValue(refToAddr(hwVersion), sizeof(Version));
    auto* swVerCharacteristic = service.createCharacteristic(
        OTA_CHARACTERISTIC_UUID_SW_VER,
        NIMBLE_PROPERTY::READ
    );
    swVerCharacteristic->setValue(refToAddr(swVersion), sizeof(Version));
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