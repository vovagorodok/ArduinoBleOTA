#pragma once
#include "BleOtaSizes.h"
#include "BleOtaStatus.h"
#include "BleOtaDefinesArduino.h"

#if defined(BLE_OTA_STATIC_BUFFER) && defined(BLE_OTA_DYNAMIC_BUFFER)
    #error "BLE_OTA_STATIC_BUFFER and BLE_OTA_DYNAMIC_BUFFER cannot be defined at the same time."
#elif defined(BLE_OTA_STATIC_BUFFER) && defined(BLE_OTA_NO_BUFFER)
    #error "BLE_OTA_STATIC_BUFFER and BLE_OTA_NO_BUFFER cannot be defined at the same time."
#elif defined(BLE_OTA_DYNAMIC_BUFFER) && defined(BLE_OTA_NO_BUFFER)
    #error "BLE_OTA_DYNAMIC_BUFFER and BLE_OTA_NO_BUFFER cannot be defined at the same time."
#endif

#if !defined(BLE_OTA_STATIC_BUFFER) && !defined(BLE_OTA_DYNAMIC_BUFFER) && !defined(BLE_OTA_NO_BUFFER)
    #if defined(BLE_OTA_NO_ALLOCATION)
        #define BLE_OTA_NO_BUFFER
    #elif defined(BLE_OTA_STATIC_ALLOCATION)
        #define BLE_OTA_STATIC_BUFFER
    #elif defined(BLE_OTA_DYNAMIC_ALLOCATION)
        #define BLE_OTA_DYNAMIC_BUFFER
    #else
        #define BLE_OTA_DYNAMIC_BUFFER
    #endif
#endif

class BleOtaBuffer
{
public:
    BleOtaBuffer();

    size_t begin(size_t bufferSize, size_t packageSize);
    BleOtaStatus push(const uint8_t* data, size_t size);
    void end();
    void clear();
    const uint8_t* data() const;
    size_t size() const;
    void setEnable(bool enable);
    bool isEnabled() const;

private:
#if defined(BLE_OTA_STATIC_BUFFER)
    uint8_t _buffer[BLE_OTA_BUFFER_SIZE];
#elif defined(BLE_OTA_DYNAMIC_BUFFER)
    uint8_t* _buffer;
#endif
#ifndef BLE_OTA_NO_BUFFER
    size_t _size;
    size_t _capacity;
    bool _enable;
#endif
};