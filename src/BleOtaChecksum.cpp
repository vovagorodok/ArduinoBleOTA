#include "BleOtaChecksum.h"
#include "BleOtaLogger.h"

namespace
{
#define TAG "Checksum"
}

#ifdef BLE_OTA_NO_CHECKSUM
BleOtaChecksum::BleOtaChecksum()
#else
BleOtaChecksum::BleOtaChecksum():
    _crc(),
    _enable(false)
#endif
{}

void BleOtaChecksum::begin()
{
#ifndef BLE_OTA_NO_CHECKSUM
    BLE_OTA_LOG(TAG, "Begin");
#endif

#if defined(BLE_OTA_CHECKSUM_LIB_CRC)
    _crc.restart();
#elif defined(BLE_OTA_CHECKSUM_LIB_MINIZ)
    _crc = 0;
#endif
}

void BleOtaChecksum::push(const uint8_t* data, size_t size)
{
#if defined(BLE_OTA_CHECKSUM_LIB_CRC)
    _crc.add(data, size);
#elif defined(BLE_OTA_CHECKSUM_LIB_MINIZ)
    _crc = mz_crc32(_crc, data, size);
#endif
}

uint32_t BleOtaChecksum::calc() const
{
#if defined(BLE_OTA_CHECKSUM_LIB_CRC)
    const auto crc = _crc.calc();
    BLE_OTA_LOG(TAG, "Calc: %lu", crc);
    return crc;
#elif defined(BLE_OTA_CHECKSUM_LIB_MINIZ)
    BLE_OTA_LOG(TAG, "Calc: %lu", _crc);
    return _crc;
#else
    return 0;
#endif
}

void BleOtaChecksum::setEnable(bool enable)
{
#ifndef BLE_OTA_NO_CHECKSUM
    BLE_OTA_LOG(TAG, "Enable: %u", enable);
    _enable = enable;
#endif
}

bool BleOtaChecksum::isEnabled() const
{
#ifndef BLE_OTA_NO_CHECKSUM
    return _enable;
#else
    return false;
#endif
}

bool BleOtaChecksum::isSupported() const
{
#ifndef BLE_OTA_NO_CHECKSUM
    return true;
#else
    return false;
#endif
}