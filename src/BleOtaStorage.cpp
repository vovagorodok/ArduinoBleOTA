#include "BleOtaStorage.h"
#include "BleOtaLogger.h"

namespace
{
#define TAG "Storage"
}

BleOtaStorage::BleOtaStorage():
    _storage(&InternalStorage),
    _size(0)
{}

void BleOtaStorage::begin(OTAStorage& storage)
{
    _storage = &storage;
}

BleOtaStatus BleOtaStorage::open(size_t firmwareSize)
{
    BLE_OTA_LOG(TAG, "Open: firmware size: %u", firmwareSize);

    const auto capacity = _storage->maxSize();
    if (capacity <= 0)
        return BleOtaStatus::InternalStorageError;
    if (firmwareSize > capacity)
        return BleOtaStatus::IncorrectFirmwareSize;

    _firmwareSize = firmwareSize;
    _size = 0;
    return _storage->open(firmwareSize) ? BleOtaStatus::Ok : BleOtaStatus::InternalStorageError;
}

BleOtaStatus BleOtaStorage::push(const uint8_t* data, size_t size)
{
    if (_size + size > _firmwareSize)
        return BleOtaStatus::IncorrectFirmwareSize;

#if defined(BLE_OTA_STORAGE_LIB_ARDUINO_OTA)
    for (size_t i = 0; i < size; i++)
    {
        _storage->write(data[i]);
    }
#else
    _storage->push(data, size);
#endif

    _size += size;
    return BleOtaStatus::Ok;
}

void BleOtaStorage::close()
{
    BLE_OTA_LOG(TAG, "Close");
    _storage->close();
}

void BleOtaStorage::apply()
{
    BLE_OTA_LOG(TAG, "Apply");
    _storage->apply();
}

void BleOtaStorage::clear()
{
    BLE_OTA_LOG(TAG, "Clear");
    _storage->clear();
    _size = 0;
}

bool BleOtaStorage::isFull() const
{
    return _size == _firmwareSize;
}

size_t BleOtaStorage::calcProgress() const
{
    return _size * 100 / _firmwareSize;
}