#pragma once
#include "BleOtaDefines.h"

// BLE packet types:
// https://developerhelp.microchip.com/wireless:ble-link-layer-packet-types
// MTU overhead:
// https://docs.silabs.com/bluetooth/4.0/general/system-and-performance/throughput-with-bluetooth-low-energy-technology#attribute-protocol-att-operation
#define BLE_OTA_MTU_WRITE_OVERHEAD 3

#ifdef BLE_OTA_BLE_LIB_NIM_BLE_ARDUINO
    #define BLE_OTA_MIN_MTU_SIZE BLE_ATT_MTU_DFLT
    #define BLE_OTA_MAX_MTU_SIZE BLE_ATT_MTU_MAX
    #define BLE_OTA_MAX_ATTR_SIZE BLE_ATT_ATTR_MAX_LEN
#else
    #define BLE_OTA_MIN_MTU_SIZE 23
    #define BLE_OTA_MAX_MTU_SIZE 242
    #define BLE_OTA_MAX_ATTR_SIZE (BLE_OTA_MAX_MTU_SIZE - BLE_OTA_MTU_WRITE_OVERHEAD)
#endif

#define BLE_OTA_HEADER_SIZE sizeof(uint8_t)
#ifndef BLE_OTA_ATTRIBUTE_SIZE
    #define BLE_OTA_ATTRIBUTE_SIZE BLE_OTA_MAX_ATTR_SIZE
#endif
#define BLE_OTA_PACKAGE_SIZE (BLE_OTA_ATTRIBUTE_SIZE - BLE_OTA_HEADER_SIZE)
#ifndef BLE_OTA_BUFFER_SIZE
    #define BLE_OTA_PACKAGES_NUM_IN_BUFFER 10
    #define BLE_OTA_BUFFER_SIZE BLE_OTA_PACKAGE_SIZE * BLE_OTA_PACKAGES_NUM_IN_BUFFER
#endif

constexpr auto BLE_OTA_ATTR_OVERHEAD = BLE_OTA_MAX_MTU_SIZE - BLE_OTA_MAX_ATTR_SIZE;
constexpr auto BLE_OTA_MTU_SIZE = BLE_OTA_ATTRIBUTE_SIZE + BLE_OTA_ATTR_OVERHEAD;

static_assert(BLE_OTA_MTU_SIZE >= BLE_OTA_MIN_MTU_SIZE,
    "OTA MTU size should be greater than minimum size.");
static_assert(BLE_OTA_MTU_SIZE <= BLE_OTA_MAX_MTU_SIZE,
    "OTA MTU size should be less than maximum size.");
