#include "BleOtaDecompressor.h"
#include "BleOtaLogger.h"
#ifdef ARDUINO_ARCH_ESP32
#include "esp_task_wdt.h"
#endif

namespace
{
#define TAG "Decompress"
}

BleOtaDecompressor::BleOtaDecompressor(BleOtaStorage& storage):
#ifndef BLE_OTA_NO_COMPRESSION
    _storage(storage),
#else
    _storage(storage)
#endif
#if defined(BLE_OTA_STATIC_COMPRESSION)
    _decompressorData{},
    _bufferData{},
    _decompressor{&_decompressorData},
    _buffer{_bufferData},
#elif defined(BLE_OTA_DYNAMIC_COMPRESSION)
    _decompressor(nullptr),
    _buffer(nullptr),
#endif
#ifndef BLE_OTA_NO_COMPRESSION
    _bufferSize(),
    _compressedSize(),
    _size(),
    _enable(true)
#endif
{}

void BleOtaDecompressor::begin(size_t compressedSize)
{
#ifndef BLE_OTA_NO_COMPRESSION
    BLE_OTA_LOG(TAG, "Begin: compressed size: %u", compressedSize);

    end();
    _bufferSize = 0;
    _compressedSize = compressedSize;
    _size = 0;
#endif

#ifdef BLE_OTA_DYNAMIC_COMPRESSION
    _decompressor = new tinfl_decompressor;
    _buffer = new uint8_t[TINFL_LZ_DICT_SIZE];
#endif

#ifndef BLE_OTA_NO_COMPRESSION
    tinfl_init(_decompressor);
#endif
}

BleOtaStatus BleOtaDecompressor::push(const uint8_t* data, size_t size)
{
#ifndef BLE_OTA_NO_COMPRESSION
    const size_t finalSize = _size + size;

    if (finalSize > _compressedSize)
    {
        BLE_OTA_LOG(TAG, "Incorrect size: total: %u, final: %u", _size, finalSize);
        return BleOtaStatus::IncorrectCompressedSize;
    }

    const bool isFinal = finalSize == _compressedSize;
    mz_uint32 decompFlags = TINFL_FLAG_PARSE_ZLIB_HEADER | TINFL_FLAG_COMPUTE_ADLER32;
    if (not isFinal)
        decompFlags |= TINFL_FLAG_HAS_MORE_INPUT;

    const uint8_t* inBufferPos = data;
    size_t inBufferAvailable = size;

    uint8_t* outBufferPos = _buffer + _bufferSize;
    size_t outBufferAvailable = TINFL_LZ_DICT_SIZE - _bufferSize;

    BLE_OTA_LOG(TAG, "Push: "
        "size: %u, is final: %u, flags: %u",
        size, isFinal, decompFlags);
    BLE_OTA_LOG(TAG, "Before: "
        "in: (avail: %u, pos: %p), "
        "out: (avail: %u, pos: %p), "
        "size: (total: %u, buff: %u)",
        inBufferAvailable, inBufferPos,
        outBufferAvailable, outBufferPos,
        _size, _bufferSize);

    while (true)
    {
        size_t inBufferSize = inBufferAvailable;
        size_t outBufferSize = outBufferAvailable;

        tinfl_status decompStatus = tinfl_decompress(
            _decompressor, inBufferPos, &inBufferSize, _buffer, outBufferPos, &outBufferSize, decompFlags);

        if (decompStatus < TINFL_STATUS_DONE)
        {
            switch (decompStatus)
            {
            case TINFL_STATUS_BAD_PARAM:
                return BleOtaStatus::IncorrectCompressionParam;
            case TINFL_STATUS_ADLER32_MISMATCH:
                return BleOtaStatus::IncorrectCompressionChecksum;
            default:
                return BleOtaStatus::IncorrectCompression;
            }
        }

        const auto status = _storage.push(outBufferPos, outBufferSize);
        if (status != BleOtaStatus::Ok)
            return status;

        inBufferPos += inBufferSize;
        inBufferAvailable -= inBufferSize;
        outBufferPos += outBufferSize;
        outBufferAvailable -= outBufferSize;
        _bufferSize += outBufferSize;
        _size += inBufferSize;

        BLE_OTA_LOG(TAG, "After: "
            "in: (avail: %u, pos: %p), "
            "out: (avail: %u, pos: %p), "
            "size: (total: %u, buff: %u, in: %u, out: %u)",
            inBufferAvailable, inBufferPos,
            outBufferAvailable, outBufferPos,
            _size, _bufferSize, inBufferSize, outBufferSize);
        BLE_OTA_LOG(TAG, "Status: %d", decompStatus);

        if (decompStatus == TINFL_STATUS_DONE or decompStatus == TINFL_STATUS_HAS_MORE_OUTPUT or not outBufferAvailable)
        {
            BLE_OTA_LOG(TAG, "Reset buffer");
            _bufferSize = 0;
            outBufferPos = _buffer;
            outBufferAvailable = TINFL_LZ_DICT_SIZE;
        }

        if (decompStatus > TINFL_STATUS_DONE and isFinal and not inBufferAvailable)
        {
            return BleOtaStatus::IncorrectCompressionEnd;
        }

        if (decompStatus >= TINFL_STATUS_DONE and not inBufferAvailable)
        {
            BLE_OTA_LOG(TAG, "Pushed");
            return BleOtaStatus::Ok;
        }

#ifdef ARDUINO_ARCH_ESP32
        esp_task_wdt_reset();
#endif
    }
#else
    return BleOtaStatus::CompressionNotSupported;
#endif
}

void BleOtaDecompressor::end()
{
#ifndef BLE_OTA_NO_COMPRESSION
    BLE_OTA_LOG(TAG, "End");
#endif

#ifdef BLE_OTA_DYNAMIC_COMPRESSION
    if (_decompressor != nullptr)
    {
        delete[] _decompressor;
        _decompressor = nullptr;
    }
    if (_buffer != nullptr)
    {
        delete[] _buffer;
        _buffer = nullptr;
    }
#endif
}

void BleOtaDecompressor::setEnable(bool enable)
{
#ifndef BLE_OTA_NO_COMPRESSION
    BLE_OTA_LOG(TAG, "Enable: %u", enable);
    _enable = enable;
#endif
}

bool BleOtaDecompressor::isEnabled() const
{
#ifndef BLE_OTA_NO_COMPRESSION
    return _enable;
#else
    return false;
#endif
}

bool BleOtaDecompressor::isSupported() const
{
#ifndef BLE_OTA_NO_COMPRESSION
    return true;
#else
    return false;
#endif
}