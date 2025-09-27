#include "BleOtaSignature.h"
#include "BleOtaLogger.h"

namespace
{
#define TAG "Signature"
}

#ifdef BLE_OTA_NO_SIGNATURE
BleOtaSignature::BleOtaSignature()
#else
BleOtaSignature::BleOtaSignature():
#endif
#if defined(BLE_OTA_STATIC_SIGNATURE)
    _sha256ContextData(),
    _pkContextData(),
    _hashData(),
    _signatureData(),
    _sha256Context(&_sha256ContextData),
    _pkContext(&_pkContextData),
    _hash(_hash),
    _signature(_signature),
#elif defined(BLE_OTA_DYNAMIC_SIGNATURE)
    _sha256Context(nullptr),
    _pkContext(nullptr),
    _hash(nullptr),
    _signature(nullptr),
#endif
#ifndef BLE_OTA_NO_SIGNATURE
    _size(),
    _enable(false)
#endif
{}

void BleOtaSignature::begin()
{
#ifndef BLE_OTA_NO_SIGNATURE
    BLE_OTA_LOG(TAG, "Begin");

    _size = 0;
#endif

#ifdef BLE_OTA_DYNAMIC_SIGNATURE
    clear();
    _sha256Context = new mbedtls_sha256_context;
    _hash = new uint8_t[BLE_OTA_SIGNATURE_HASH_SIZE];
    _signature = new uint8_t[BLE_OTA_SIGNATURE_SIZE];
#endif

#ifndef BLE_OTA_NO_SIGNATURE
    mbedtls_sha256_init(_sha256Context);
    mbedtls_sha256_starts(_sha256Context, 0);
#endif
}

void BleOtaSignature::push(const uint8_t* data, size_t size)
{
#ifndef BLE_OTA_NO_SIGNATURE
    mbedtls_sha256_update(_sha256Context, static_cast<const unsigned char*>(data), size);
#endif
}

BleOtaStatus BleOtaSignature::pushSignature(const uint8_t* data, size_t size)
{
#ifndef BLE_OTA_NO_SIGNATURE
    const size_t finalSize = _size + size;

    if (finalSize > BLE_OTA_SIGNATURE_SIZE)
    {
        BLE_OTA_LOG(TAG, "Incorrect signature size: total: %u, final: %u", _size, finalSize);
        return BleOtaStatus::IncorrectSignatureSize;
    }

    memcpy(_signature + _size, data, size);
    _size += size;

    return BleOtaStatus::Ok;
#else
    return BleOtaStatus::SignatureNotSupported;
#endif
}

BleOtaStatus BleOtaSignature::end()
{
#ifndef BLE_OTA_NO_SIGNATURE
    BLE_OTA_LOG(TAG, "End");

    if (_size != BLE_OTA_SIGNATURE_SIZE)
    {
        BLE_OTA_LOG(TAG, "Incorrect signature size: %u", _size);
        return BleOtaStatus::IncorrectSignatureSize;
    }

    _size = 0;
    mbedtls_sha256_finish(_sha256Context, _hash);
    mbedtls_sha256_free(_sha256Context);

    const bool success = mbedtls_pk_verify(
        _pkContext,
        MBEDTLS_MD_SHA256,
        _hash,
        BLE_OTA_SIGNATURE_HASH_SIZE,
        _signature,
        BLE_OTA_SIGNATURE_SIZE) == 0;

#ifdef BLE_OTA_DYNAMIC_SIGNATURE
    clear();
#endif

    return success ? BleOtaStatus::Ok : BleOtaStatus::IncorrectSignature;
#else
    return BleOtaStatus::SignatureNotSupported;
#endif
}

bool BleOtaSignature::setPublicKey(const char* key, size_t size)
{
#ifndef BLE_OTA_NO_SIGNATURE
    if (size < BLE_OTA_SIGNATURE_SIZE)
    {
        BLE_OTA_LOG(TAG, "Incorrect public key size: %u", size);
        return false;
    }

    if (_enable)
    {
        mbedtls_pk_free(_pkContext);
    }

#ifdef BLE_OTA_DYNAMIC_SIGNATURE
    if (_pkContext != nullptr)
    {
        delete _pkContext;
        _pkContext = nullptr;
    }
    _pkContext = new mbedtls_pk_context;
#endif

    mbedtls_pk_init(_pkContext);

    const auto* pKey = reinterpret_cast<const unsigned char*>(key);
    const auto pSize = size + 1;
    _enable = mbedtls_pk_parse_public_key(_pkContext, pKey, pSize) == 0;

    if (not _enable)
    {
        mbedtls_pk_free(_pkContext);
#ifdef BLE_OTA_DYNAMIC_SIGNATURE
        delete _pkContext;
        _pkContext = nullptr;
#endif
    }

    BLE_OTA_LOG(TAG, "Public key enable: %u", _enable);
    return _enable;
#else
    return false;
#endif
}

bool BleOtaSignature::isEnabled() const
{
#ifndef BLE_OTA_NO_SIGNATURE
    return _enable;
#else
    return false;
#endif
}

void BleOtaSignature::clear()
{
#ifdef BLE_OTA_DYNAMIC_SIGNATURE
    if (_sha256Context != nullptr)
    {
        delete _sha256Context;
        _sha256Context = nullptr;
    }
    if (_hash != nullptr)
    {
        delete[] _hash;
        _hash = nullptr;
    }
    if (_signature != nullptr)
    {
        delete[] _signature;
        _signature = nullptr;
    }
#endif
}