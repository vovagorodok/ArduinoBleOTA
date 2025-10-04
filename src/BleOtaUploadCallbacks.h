#pragma once
#include "BleOtaStatus.h"

class BleOtaUploadCallbacks
{
public:
    virtual void handleUploadBegin() {}
    virtual void handleUploadProgress(size_t percentages) {}
    virtual void handleUploadEnd() {}
    virtual void handleUploadError(BleOtaStatus errorCode) {}

    virtual ~BleOtaUploadCallbacks() = default;
};