#pragma once
#include "BleOtaStorage.h"
#include "BleOtaVersion.h"
#include <ArduinoBLE.h>

class BleOtaUploader;

class ArduinoBleOTAClass
{
public:
    bool begin(const String& deviceName, OTAStorage& storage);
    bool begin(OTAStorage& storage);
    bool begin(const String& deviceName, OTAStorage& storage,
               const String& hwName, Version hwVersion,
               const String& swName, Version swVersion);
    bool begin(OTAStorage& storage,
               const String& hwName, Version hwVersion,
               const String& swName, Version swVersion);
    void pull();

private:
    friend BleOtaUploader;
    void begin(const String &hwName, Version hwVersion,
               const String &swName, Version swVersion);
    void send(const uint8_t* data, size_t length);
};

extern ArduinoBleOTAClass ArduinoBleOTA;