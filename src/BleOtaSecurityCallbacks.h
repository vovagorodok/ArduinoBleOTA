#pragma once
#include <Arduino.h>

class BleOtaSecurityCallbacks
{
public:
    virtual bool setPin(uint32_t pin)
    {
        return false;
    }

    virtual bool removePin()
    {
        return false;
    }
};