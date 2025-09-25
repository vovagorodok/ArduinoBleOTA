#pragma once
#include "BleOtaStatus.h"

class BleOtaUploadCallbacks
{
public:
    virtual void handleBegin() {}
    virtual void handleEnd() {}
    virtual void handleError(BleOtaStatus errorCode) {}
};