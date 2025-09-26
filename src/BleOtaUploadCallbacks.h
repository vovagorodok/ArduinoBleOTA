#pragma once
#include "BleOtaStatus.h"

class BleOtaUploadCallbacks
{
public:
    virtual void handleUploadBegin() {}
    virtual void handleUploadEnd() {}
    virtual void handleUploadError(BleOtaStatus errorCode) {}
};