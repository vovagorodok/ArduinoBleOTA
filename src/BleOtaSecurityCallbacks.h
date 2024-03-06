#pragma once
#include <Arduino.h>

class BleOtaSecurityCallbacks
{
public:
    virtual bool setPinCode(uint32_t pinCode)
    {
        return false;
    }

    virtual bool removePinCode()
    {
        return false;
    }
};