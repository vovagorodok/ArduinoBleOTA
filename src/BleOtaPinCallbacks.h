#pragma once
#include <Arduino.h>

class BleOtaPinCallbacks
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

    virtual ~BleOtaPinCallbacks() = default;
};