#pragma once
#include "BleOtaStatus.h"

#if !defined(BLE_OTA_NO_SIGNATURE)
    #if __has_include("mbedtls/sha256.h") && __has_include("mbedtls/pk.h")
        #include "mbedtls/sha256.h"
        #include "mbedtls/pk.h"
        #define BLE_OTA_SIGNATURE_LIB_MBEDTLS
    #else
        #define BLE_OTA_NO_SIGNATURE
    #endif
#endif

#if defined(BLE_OTA_STATIC_SIGNATURE) && defined(BLE_OTA_DYNAMIC_SIGNATURE)
    #error "BLE_OTA_STATIC_SIGNATURE and BLE_OTA_DYNAMIC_SIGNATURE cannot be defined at the same time."
#elif defined(BLE_OTA_STATIC_SIGNATURE) && defined(BLE_OTA_NO_SIGNATURE)
    #error "BLE_OTA_STATIC_SIGNATURE and BLE_OTA_NO_SIGNATURE cannot be defined at the same time."
#elif defined(BLE_OTA_DYNAMIC_SIGNATURE) && defined(BLE_OTA_NO_SIGNATURE)
    #error "BLE_OTA_DYNAMIC_SIGNATURE and BLE_OTA_NO_SIGNATURE cannot be defined at the same time."
#endif

#if !defined(BLE_OTA_STATIC_SIGNATURE) && !defined(BLE_OTA_DYNAMIC_SIGNATURE) && !defined(BLE_OTA_NO_SIGNATURE)
    #if defined(BLE_OTA_NO_ALLOCATION)
        #define BLE_OTA_NO_SIGNATURE
    #elif defined(BLE_OTA_STATIC_ALLOCATION)
        #define BLE_OTA_STATIC_SIGNATURE
    #elif defined(BLE_OTA_DYNAMIC_ALLOCATION)
        #define BLE_OTA_DYNAMIC_SIGNATURE
    #else
        #define BLE_OTA_DYNAMIC_SIGNATURE
    #endif
#endif

#ifndef BLE_OTA_NO_SIGNATURE
#define BLE_OTA_SIGNATURE_SIZE 256
#define BLE_OTA_SIGNATURE_HASH_SIZE 32
#endif

class BleOtaSignature
{
public:
    BleOtaSignature();

    void begin();
    void push(const uint8_t* data, size_t size);
    BleOtaStatus pushSignature(const uint8_t* data, size_t size);
    BleOtaStatus end();
    void clear();
    bool setPublicKey(const char* key, size_t size);
    bool isEnabled() const;

private:
#if defined(BLE_OTA_STATIC_SIGNATURE)
    mbedtls_sha256_context _sha256ContextData;
    mbedtls_pk_context _pkContextData;
    uint8_t _hashData[BLE_OTA_SIGNATURE_HASH_SIZE];
    uint8_t _signatureData[BLE_OTA_SIGNATURE_HASH_SIZE];
#endif
#ifndef BLE_OTA_NO_SIGNATURE
    mbedtls_sha256_context* _sha256Context;
    mbedtls_pk_context* _pkContext;
    uint8_t* _hash;
    uint8_t* _signature;
    size_t _size;
    bool _enable;
#endif
};