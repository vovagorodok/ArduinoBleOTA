#pragma once
#include "BleOtaStatus.h"
#include "BleOtaPercentages.h"

class BleOtaUploadCallbacks
{
public:
    virtual void handleUploadBegin() {}
    virtual void handleUploadProgress(BleOtaPercentages progress) {}
    virtual void handleUploadEnd() {}
    virtual void handleUploadError(BleOtaStatus errorCode) {}

    virtual ~BleOtaUploadCallbacks() = default;
};