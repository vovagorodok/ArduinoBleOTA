#pragma once
#include "BleOtaVersion.h"

struct BleOtaInfo
{
    const char* mfName;
    const char* hwName;
    const char* swName;
    const BleOtaVersion hwVersion;
    const BleOtaVersion swVersion;

    BleOtaInfo(const char* mfName = " ",
               const char* hwName = " ",
               const char* swName = " ",
               const BleOtaVersion hwVersion = {},
               const BleOtaVersion swVersion = {}):
        mfName(mfName),
        hwName(hwName),
        swName(swName),
        hwVersion(hwVersion),
        swVersion(swVersion)
    {}
};