#pragma once
#include <Arduino.h>

template <typename T>
inline const T& addrToRef(const uint8_t* addr)
{
    return *reinterpret_cast<const T*>(addr);
}

template <typename T>
inline const uint8_t* refToAddr(const T& value)
{
    return reinterpret_cast<const uint8_t*>(&value);
}