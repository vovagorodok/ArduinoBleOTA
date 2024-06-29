#pragma once
#include "BleOtaStorage.h"
#include "BleOtaVersion.h"
#include "BleOtaSecurityCallbacks.h"
#include "BleOtaUploadCallbacks.h"
#include <ArduinoBLE.h>

class BleOtaUploader;

class ArduinoBleOTAClass
{
public:
    ArduinoBleOTAClass();

    bool begin(const String& deviceName,
               OTAStorage& storage,
               const String& mfName = " ",
               const String& hwName = " ",
               BleOtaVersion hwVersion = {},
               const String& swName = " ",
               BleOtaVersion swVersion = {},
               bool enableUpload = true);
    bool begin(OTAStorage& storage,
               const String& mfName = " ",
               const String& hwName = " ",
               BleOtaVersion hwVersion = {},
               const String& swName = " ",
               BleOtaVersion swVersion = {},
               bool enableUpload = true);
    void pull();

    void enableUpload();
    void disableUpload();
    void setSecurityCallbacks(BleOtaSecurityCallbacks&);
    void setUploadCallbacks(BleOtaUploadCallbacks&);

private:
    friend BleOtaUploader;
    void begin(const String& mfName,
               const String& hwName, BleOtaVersion hwVersion,
               const String& swName, BleOtaVersion swVersion);
    void send(const uint8_t* data, size_t length);

    BleOtaSecurityCallbacks* securityCallbacks;
    BleOtaUploadCallbacks* uploadCallbacks;
};

extern ArduinoBleOTAClass ArduinoBleOTA;