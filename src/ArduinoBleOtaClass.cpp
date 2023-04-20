#ifndef USE_NIM_BLE_ARDUINO_LIB
#include "ArduinoBleOtaClass.h"
#include "BleOtaUploader.h"
#include "BleOtaUuids.h"
#include "BleOtaSizes.h"

namespace
{
BLEService service(BLE_OTA_SERVICE_UUID);
BLECharacteristic rxCharacteristic(BLE_OTA_CHARACTERISTIC_UUID_RX, BLEWriteWithoutResponse, BLE_OTA_MAX_ATTR_SIZE);
BLECharacteristic txCharacteristic(BLE_OTA_CHARACTERISTIC_UUID_TX, BLERead | BLENotify, BLE_OTA_MAX_ATTR_SIZE);
BLEStringCharacteristic hwNameCharacteristic(BLE_OTA_CHARACTERISTIC_UUID_HW_NAME, BLERead, BLE_OTA_MAX_ATTR_SIZE);
BLEStringCharacteristic swNameCharacteristic(BLE_OTA_CHARACTERISTIC_UUID_SW_NAME, BLERead, BLE_OTA_MAX_ATTR_SIZE);
BLECharacteristic hwVerCharacteristic(BLE_OTA_CHARACTERISTIC_UUID_HW_VER, BLERead, sizeof(BleOtaVersion), true);
BLECharacteristic swVerCharacteristic(BLE_OTA_CHARACTERISTIC_UUID_SW_VER, BLERead, sizeof(BleOtaVersion), true);

static BleOtaSecurity dummySecurity{};

void onWrite(BLEDevice central, BLECharacteristic characteristic)
{
    bleOtaUploader.onData(rxCharacteristic.value(), rxCharacteristic.valueLength());
}
}

ArduinoBleOTAClass::ArduinoBleOTAClass() :
    security(&dummySecurity)
{}

bool ArduinoBleOTAClass::begin(const String& deviceName, OTAStorage& storage,
                               const String& hwName, BleOtaVersion hwVersion,
                               const String& swName, BleOtaVersion swVersion,
                               bool enableUpload)
{
    if (!BLE.begin())
        return false;

    BLE.setLocalName(deviceName.c_str());
    BLE.setDeviceName(deviceName.c_str());

    if(!begin(storage, hwName, hwVersion, swName, swVersion, enableUpload))
        return false;

    return BLE.advertise();
}

bool ArduinoBleOTAClass::begin(OTAStorage& storage,
                               const String& hwName, BleOtaVersion hwVersion,
                               const String& swName, BleOtaVersion swVersion,
                               bool enableUpload)
{
    bleOtaUploader.begin(storage);
    bleOtaUploader.setEnabling(enableUpload);
    service.addCharacteristic(rxCharacteristic);
    service.addCharacteristic(txCharacteristic);
    rxCharacteristic.setEventHandler(BLEWritten, onWrite);

    begin(hwName, hwVersion, swName, swVersion);

    BLE.addService(service);

    return BLE.setAdvertisedService(service);
}

void ArduinoBleOTAClass::begin(const String& hwName, BleOtaVersion hwVersion,
                               const String& swName, BleOtaVersion swVersion)
{
    service.addCharacteristic(hwNameCharacteristic);
    hwNameCharacteristic.setValue(hwName);
    service.addCharacteristic(swNameCharacteristic);
    swNameCharacteristic.setValue(swName);
    service.addCharacteristic(hwVerCharacteristic);
    hwVerCharacteristic.setValue((const uint8_t*)(&hwVersion), sizeof(BleOtaVersion));
    service.addCharacteristic(swVerCharacteristic);
    swVerCharacteristic.setValue((const uint8_t*)(&swVersion), sizeof(BleOtaVersion));
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

void ArduinoBleOTAClass::setSecurity(BleOtaSecurity& callbacks)
{
    security = &callbacks;
}

void ArduinoBleOTAClass::send(const uint8_t* data, size_t length)
{
    txCharacteristic.setValue(data, length);
}

ArduinoBleOTAClass ArduinoBleOTA{};
#endif