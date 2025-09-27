#pragma once
#include <Arduino.h>

#pragma pack(push, 1)
struct BleOtaVersion
{
    constexpr BleOtaVersion():
        major(),
        minor(),
        patch()
    {}
    constexpr BleOtaVersion(uint8_t major, uint8_t minor, uint8_t patch):
        major(major),
        minor(minor),
        patch(patch)
    {}

private:
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
};
#pragma pack(pop)