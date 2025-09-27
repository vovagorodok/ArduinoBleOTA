#pragma once
#include "BleOtaPinCallbacks.h"
#ifdef BLE_OTA_BLE_LIB_NIM_BLE_ARDUINO
#include <NimBLEDevice.h>
#include <Preferences.h>

#define BLE_OTA_SECURITY_DICT "ota_security"
#define BLE_OTA_PIN_CODE_KEY "ota_pin"

class BleOtaSecurityServer: public BleOtaPinCallbacks,
                            public BLEServerCallbacks
{
public:
    void begin()
    {
        if (not prefs.begin(BLE_OTA_SECURITY_DICT))
            return;

        if (not prefs.isKey(BLE_OTA_PIN_CODE_KEY))
        {
            prefs.end();
            return;
        }

        BLEDevice::setSecurityPasskey(prefs.getUInt(BLE_OTA_PIN_CODE_KEY));
        BLEDevice::setSecurityAuth(true, true, true);
        BLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);
        auto* server = BLEDevice::createServer();
        server->setCallbacks(this);
        prefs.end();
    }

    bool setPinCode(uint32_t pinCode) override
    {
        if (not prefs.begin(BLE_OTA_SECURITY_DICT))
            return false;

        const bool result = prefs.putUInt(BLE_OTA_PIN_CODE_KEY, pinCode);

        prefs.end();
        return result;
    }

    bool removePinCode() override
    {
        if (not prefs.begin(BLE_OTA_SECURITY_DICT))
            return false;

        const bool result = not prefs.isKey(BLE_OTA_PIN_CODE_KEY) or
                            prefs.remove(BLE_OTA_PIN_CODE_KEY);

        prefs.end();
        return result;
    }

    void onConnect(BLEServer* srv, ble_gap_conn_desc* desc) override
    {
        BLEDevice::startSecurity(desc->conn_handle);
    }

private:
    Preferences prefs;
};

#endif