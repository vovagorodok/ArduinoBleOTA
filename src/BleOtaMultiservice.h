#pragma once
#include <ArduinoBleOTA.h>
#include "BleOtaUuids.h"

#ifdef USE_ARDUINO_BLE_LIB
inline bool initBle(const char* deviceName)
{
    if (!BLE.begin())
        return false;

    BLE.setLocalName(deviceName);
    BLE.setDeviceName(deviceName);
    return true;
}

inline bool advertiseBle(const char* deviceName,
                         const char* primaryUUID,
                         const char* secondaryUUID)
{
    BLEAdvertisingData primaryAdvertisementData{};
    primaryAdvertisementData.setFlags(BLEFlagsGeneralDiscoverable | BLEFlagsBREDRNotSupported);
    primaryAdvertisementData.setAdvertisedServiceUuid(primaryUUID);
    BLE.setAdvertisingData(primaryAdvertisementData);

    BLEAdvertisingData secondaryAdvertisementData{};
    secondaryAdvertisementData.setLocalName(deviceName);
    secondaryAdvertisementData.setAdvertisedServiceUuid(secondaryUUID);
    BLE.setScanResponseData(secondaryAdvertisementData);

    return BLE.advertise();
}

inline bool advertiseBle(const char* deviceName,
                         const char* secondaryUUID)
{
    return advertiseBle(deviceName, BLE_OTA_SERVICE_UUID, secondaryUUID);
}
#else
inline void initBle(const std::string& deviceName)
{
    BLEDevice::init(deviceName);
}

inline bool advertiseBle(const std::string& deviceName,
                         const std::string& primaryUUID,
                         const std::string& secondaryUUID)
{
    auto* server = BLEDevice::createServer();
    auto* advertising = server->getAdvertising();

    BLEAdvertisementData primaryAdvertisementData{};
    #ifdef USE_NIM_BLE_ARDUINO_LIB
    primaryAdvertisementData.setFlags(BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP);
    #else
    primaryAdvertisementData.setFlags(ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT);
    #endif
    primaryAdvertisementData.setCompleteServices(BLEUUID(primaryUUID));
    advertising->setAdvertisementData(primaryAdvertisementData);

    BLEAdvertisementData secondaryAdvertisementData{};
    secondaryAdvertisementData.setShortName(deviceName);
    secondaryAdvertisementData.setCompleteServices(BLEUUID(secondaryUUID));    
    advertising->setScanResponseData(secondaryAdvertisementData);

    #ifdef USE_NIM_BLE_ARDUINO_LIB
    return advertising->start();
    #else
    return true;
    #endif
}

inline bool advertiseBle(const std::string& deviceName,
                         const std::string& secondaryUUID)
{
    return advertiseBle(deviceName, BLE_OTA_SERVICE_UUID, secondaryUUID);
}
#endif