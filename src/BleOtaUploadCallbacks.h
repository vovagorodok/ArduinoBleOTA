#pragma once
#include <Arduino.h>

class BleOtaUploadCallbacks
{
public:
    virtual void onBegin(uint32_t firmwareLength) {}
    virtual void onEnd() {}
    virtual void onError(uint8_t errorCode) {}
};