#pragma once
#ifndef USE_ESP32_BLE_LIB
#include "BleOtaStorage.h"
#include "BleOtaVersion.h"
#include "BleOtaSecurity.h"
#include <ArduinoBLE.h>

class BleOtaUploader;

class ArduinoBleOTAClass
{
public:
    ArduinoBleOTAClass();

    bool begin(const String& deviceName,
               OTAStorage& storage,
               const String& hwName = {},
               BleOtaVersion hwVersion = {},
               const String& swName = {},
               BleOtaVersion swVersion = {},
               bool enableUpload = true);
    bool begin(OTAStorage& storage,
               const String& hwName = {},
               BleOtaVersion hwVersion = {},
               const String& swName = {},
               BleOtaVersion swVersion = {},
               bool enableUpload = true);
    void pull();

    void enableUpload();
    void disableUpload();
    void setSecurity(BleOtaSecurity& callbacks);

private:
    friend BleOtaUploader;
    void begin(const String &hwName, BleOtaVersion hwVersion,
               const String &swName, BleOtaVersion swVersion);
    void send(const uint8_t* data, size_t length);

    BleOtaSecurity* security;
};

extern ArduinoBleOTAClass ArduinoBleOTA;
#endif