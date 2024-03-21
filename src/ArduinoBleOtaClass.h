#pragma once
#include "BleOtaStorage.h"
#include "BleOtaVersion.h"
#include "BleOtaSecurityCallbacks.h"
#include "BleOtaUploadCallbacks.h"
#ifdef ARDUINO_ARCH_STM32
#include <STM32duinoBLE.h>
#else
#include <ArduinoBLE.h>
#endif

class BleOtaUploader;

class ArduinoBleOTAClass
{
public:
    ArduinoBleOTAClass();

    bool begin(const String& deviceName,
               OTAStorage& storage,
               const String& hwName = " ",
               BleOtaVersion hwVersion = {},
               const String& swName = " ",
               BleOtaVersion swVersion = {},
               bool enableUpload = true);
    bool begin(OTAStorage& storage,
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
    void begin(const String &hwName, BleOtaVersion hwVersion,
               const String &swName, BleOtaVersion swVersion);
    void send(const uint8_t* data, size_t length);

    BleOtaSecurityCallbacks* securityCallbacks;
    BleOtaUploadCallbacks* uploadCallbacks;
};

extern ArduinoBleOTAClass ArduinoBleOTA;