#pragma once
#include <Arduino.h>

#pragma pack(push, 1)
struct BleOtaVersion
{
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
};
#pragma pack(pop)