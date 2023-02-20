#pragma once
#include <ArduinoBleOTA.h>

#if defined(USE_NIM_BLE_ARDUINO_LIB)
    #define BLE_OTA_MIN_MTU_SIZE BLE_ATT_MTU_DFLT
    #define BLE_OTA_MAX_MTU_SIZE BLE_ATT_MTU_MAX
    #define BLE_OTA_MAX_ATTR_SIZE BLE_ATT_ATTR_MAX_LEN
    #define BLE_OTA_PACKAGES_NUM_IN_BUFFER 10
#else
    #define BLE_OTA_MIN_MTU_SIZE 23
    #define BLE_OTA_MAX_MTU_SIZE 143
    #define BLE_OTA_MAX_ATTR_SIZE 128
    #define BLE_OTA_PACKAGES_NUM_IN_BUFFER 40
#endif

#define BLE_OTA_HEADER_SIZE sizeof(uint8_t)
#ifndef BLE_OTA_ATTRIBUTE_SIZE
    #define BLE_OTA_ATTRIBUTE_SIZE BLE_OTA_MAX_ATTR_SIZE
#endif
#ifndef BLE_OTA_BUFFER_SIZE
    #define BLE_OTA_BUFFER_SIZE (BLE_OTA_ATTRIBUTE_SIZE - BLE_OTA_HEADER_SIZE) * BLE_OTA_PACKAGES_NUM_IN_BUFFER
#endif

constexpr auto BLE_OTA_ATTR_OVERHEAD = BLE_OTA_MAX_MTU_SIZE - BLE_OTA_MAX_ATTR_SIZE;
constexpr auto BLE_OTA_MTU_SIZE = BLE_OTA_ATTRIBUTE_SIZE + BLE_OTA_ATTR_OVERHEAD;

static_assert(BLE_OTA_MTU_SIZE >= BLE_OTA_MIN_MTU_SIZE,
    "OTA MTU size should be greater than minimum size.");
static_assert(BLE_OTA_MTU_SIZE <= BLE_OTA_MAX_MTU_SIZE,
    "OTA MTU size should be less than maximum size.");
