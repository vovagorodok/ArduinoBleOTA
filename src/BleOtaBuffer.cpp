#include "BleOtaBuffer.h"

#ifndef BLE_OTA_NO_BUFFER
BleOtaBuffer::BleOtaBuffer():
#else
BleOtaBuffer::BleOtaBuffer()
#endif
#if defined(BLE_OTA_STATIC_BUFFER)
    _buffer(),
#elif defined(BLE_OTA_DYNAMIC_BUFFER)
    _buffer(nullptr),
#endif
#ifndef BLE_OTA_NO_BUFFER
    _size(),
    _capacity(),
    _enable(true)
#endif
{}

size_t BleOtaBuffer::begin(size_t bufferSize, size_t packageSize)
{
#ifndef BLE_OTA_NO_BUFFER
    end();

    if (not _enable)
    {
        return 0;
    }

    _capacity = min(bufferSize, static_cast<size_t>(BLE_OTA_BUFFER_SIZE));

    if (_capacity < packageSize)
    {
        _enable = false;
        return 0;
    }
#endif

#ifdef BLE_OTA_DYNAMIC_BUFFER
    _buffer = new uint8_t[_capacity];
#endif

#ifndef BLE_OTA_NO_BUFFER
    return _capacity;
#else
    return 0;
#endif
}

BleOtaStatus BleOtaBuffer::push(const uint8_t* data, size_t size)
{
#ifndef BLE_OTA_NO_BUFFER
    if (_size + size > _capacity)
        return BleOtaStatus::BufferOverflow;
    memcpy(_buffer + _size, data, size);
    _size += size;
    return BleOtaStatus::Ok;
#else
    return BleOtaStatus::BufferDisabled;
#endif
}

void BleOtaBuffer::end()
{
#ifndef BLE_OTA_NO_BUFFER
    clear();
#endif

#ifdef BLE_OTA_DYNAMIC_BUFFER
    delete[] _buffer;
    _buffer = nullptr;
#endif
}

void BleOtaBuffer::clear()
{
#ifndef BLE_OTA_NO_BUFFER
    _size = 0;
#endif
}

const uint8_t* BleOtaBuffer::data() const
{
#ifndef BLE_OTA_NO_BUFFER
    return _buffer;
#else
    return nullptr;
#endif
}

size_t BleOtaBuffer::size() const
{
#ifndef BLE_OTA_NO_BUFFER
    return _size;
#else
    return 0;
#endif
}

void BleOtaBuffer::setEnable(bool enable)
{
#ifndef BLE_OTA_NO_BUFFER
    _enable = enable;
#endif
}

bool BleOtaBuffer::isEnabled() const
{
#ifndef BLE_OTA_NO_BUFFER
    return _enable;
#else
    return false;
#endif
}