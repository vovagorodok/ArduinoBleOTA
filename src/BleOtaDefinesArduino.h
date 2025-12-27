#pragma once

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <stddef.h>

using namespace std;

inline void delay(unsigned long ms) {}
#endif