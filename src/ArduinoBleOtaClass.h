#pragma once
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
               bool enable = true);
    bool begin(OTAStorage& storage,
               const String& hwName = {},
               BleOtaVersion hwVersion = {},
               const String& swName = {},
               BleOtaVersion swVersion = {},
               bool enable = true);
    void pull();

    void enable();
    void disable();
    void setSecurity(BleOtaSecurity& callbacks);

private:
    friend BleOtaUploader;
    void begin(const String &hwName, BleOtaVersion hwVersion,
               const String &swName, BleOtaVersion swVersion);
    void send(const uint8_t* data, size_t length);

    BleOtaSecurity* security;
};

extern ArduinoBleOTAClass ArduinoBleOTA;