#pragma once
#include "BleOtaStatus.h"

enum BleOtaHeader: uint8_t
{
    InitReq = 0x01,
    InitResp = 0x02,
    BeginReq = 0x03,
    BeginResp = 0x04,
    PackageInd = 0x05,
    PackageReq = 0x06,
    PackageResp = 0x07,
    EndReq = 0x08,
    EndResp = 0x09,
    ErrorInd = 0x10,
    UploadEnableInd = 0x11,
    UploadDisableInd = 0x12,
    SignatureReq = 0x20,
    SignatureResp = 0x21,
    SetPinReq = 0x30,
    SetPinResp = 0x31,
    RemovePinReq = 0x32,
    RemovePinResp = 0x33,
};

struct BleOtaMessage
{
    BleOtaMessage(BleOtaHeader header):
        header(header)
    {}
    BleOtaMessage(const uint8_t* data):
        header(static_cast<BleOtaHeader>(*data))
    {}
    static bool isValidSize(size_t size)
    {
        return size >= sizeof(BleOtaHeader);
    }

    BleOtaHeader header;
};

struct BleOtaInitReq: public BleOtaMessage
{
    BleOtaInitReq():
        BleOtaMessage(BleOtaHeader::InitReq)
    {}
    static bool isValidSize(size_t size)
    {
        return size == sizeof(BleOtaHeader);
    }
};

struct BleOtaInitResp: public BleOtaMessage
{
    struct EnableFlags {
        bool compression: 1;
        bool checksum: 1;
        bool upload: 1;
        bool signature: 1;
        bool pin: 1;
    };

    BleOtaInitResp(EnableFlags flags):
        BleOtaMessage(BleOtaHeader::InitResp),
        flags(flags)
    {}

private:
    EnableFlags flags;
};

struct BleOtaBeginReq: public BleOtaMessage
{
    struct EnableFlags {
        bool compression: 1;
        bool checksum: 1;
    };

private:
#pragma pack(push, 1)
    struct Packed {
        uint32_t firmwareSize;
        uint32_t packageSize;
        uint32_t bufferSize;
        uint32_t compressedSize;
        EnableFlags flags;
    };
#pragma pack(pop)
    BleOtaBeginReq(Packed packed):
        BleOtaMessage(BleOtaHeader::BeginReq),
        firmwareSize(packed.firmwareSize),
        packageSize(packed.packageSize),
        bufferSize(packed.bufferSize),
        compressedSize(packed.compressedSize),
        flags(packed.flags)
    {}
    static Packed pack(const uint8_t* data)
    {
        // Use copies in order to prevent hard faults caused by unaligned 32-bit accesses on Cortex-M
        Packed packed;
        memcpy(&packed, data + sizeof(BleOtaHeader), sizeof(Packed));
        return packed;
    }

public:
    BleOtaBeginReq(const uint8_t* data):
        BleOtaBeginReq(pack(data))
    {}
    static bool isValidSize(size_t size)
    {
        return size == sizeof(BleOtaHeader) + sizeof(Packed);
    }

    uint32_t firmwareSize;
    uint32_t packageSize;
    uint32_t bufferSize;
    uint32_t compressedSize;
    EnableFlags flags;
};

#pragma pack(push, 1)
struct BleOtaBeginResp: public BleOtaMessage
{
    BleOtaBeginResp(uint32_t packageSize, uint32_t bufferSize):
        BleOtaMessage(BleOtaHeader::BeginResp),
        packageSize(packageSize),
        bufferSize(bufferSize)
    {}

private:
    uint32_t packageSize;
    uint32_t bufferSize;
};
#pragma pack(pop)

struct BleOtaPackage: public BleOtaMessage
{
    BleOtaPackage(BleOtaHeader header, const uint8_t* data, size_t size):
        BleOtaMessage(header),
        data(data + sizeof(BleOtaHeader)),
        size(size - sizeof(BleOtaHeader))
    {}

    const uint8_t* data;
    size_t size;
};

struct BleOtaPackageInd: public BleOtaPackage
{
    BleOtaPackageInd(const uint8_t* data, size_t size):
        BleOtaPackage(BleOtaHeader::PackageInd, data, size)
    {}
};

struct BleOtaPackageReq: public BleOtaPackage
{
    BleOtaPackageReq(const uint8_t* data, size_t size):
        BleOtaPackage(BleOtaHeader::PackageReq, data, size)
    {}
};

struct BleOtaPackageResp: public BleOtaMessage
{
    BleOtaPackageResp():
        BleOtaMessage(BleOtaHeader::PackageResp)
    {}
};

struct BleOtaEndReq: public BleOtaMessage
{
private:
#pragma pack(push, 1)
    struct Packed {
        uint32_t firmwareCrc;
    };
#pragma pack(pop)
    BleOtaEndReq(Packed packed):
        BleOtaMessage(BleOtaHeader::EndReq),
        firmwareCrc(packed.firmwareCrc)
    {}
    static Packed pack(const uint8_t* data)
    {
        Packed packed;
        memcpy(&packed, data + sizeof(BleOtaHeader), sizeof(Packed));
        return packed;
    }

public:
    BleOtaEndReq(const uint8_t* data):
        BleOtaEndReq(pack(data))
    {}
    static bool isValidSize(size_t size)
    {
        return size == sizeof(BleOtaHeader) + sizeof(Packed);
    }

    uint32_t firmwareCrc;
};

struct BleOtaEndResp: public BleOtaMessage
{
    BleOtaEndResp():
        BleOtaMessage(BleOtaHeader::EndResp)
    {}
};

struct BleOtaErrorInd: public BleOtaMessage
{
    BleOtaErrorInd(BleOtaStatus code):
        BleOtaMessage(BleOtaHeader::ErrorInd),
        code(code)
    {}

private:
    BleOtaStatus code;
};

struct BleOtaUploadEnableInd: public BleOtaMessage
{
    BleOtaUploadEnableInd():
        BleOtaMessage(BleOtaHeader::UploadEnableInd)
    {}
};

struct BleOtaUploadDisableInd: public BleOtaMessage
{
    BleOtaUploadDisableInd():
        BleOtaMessage(BleOtaHeader::UploadDisableInd)
    {}
};

struct BleOtaSignatureReq: public BleOtaPackage
{
    BleOtaSignatureReq(const uint8_t* data, size_t size):
        BleOtaPackage(BleOtaHeader::SignatureReq, data, size)
    {}
};

struct BleOtaSignatureResp: public BleOtaMessage
{
    BleOtaSignatureResp():
        BleOtaMessage(BleOtaHeader::SignatureResp)
    {}
};

struct BleOtaSetPinReq: public BleOtaMessage
{
private:
#pragma pack(push, 1)
    struct Packed {
        uint32_t pin;
    };
#pragma pack(pop)
    BleOtaSetPinReq(Packed packed):
        BleOtaMessage(BleOtaHeader::SetPinReq),
        pin(packed.pin)
    {}
    static Packed pack(const uint8_t* data)
    {
        Packed packed;
        memcpy(&packed, data + sizeof(BleOtaHeader), sizeof(Packed));
        return packed;
    }

public:
    BleOtaSetPinReq(const uint8_t* data):
        BleOtaSetPinReq(pack(data))
    {}
    static bool isValidSize(size_t size)
    {
        return size == sizeof(BleOtaHeader) + sizeof(Packed);
    }

    uint32_t pin;
};

struct BleOtaSetPinResp: public BleOtaMessage
{
    BleOtaSetPinResp():
        BleOtaMessage(BleOtaHeader::SetPinResp)
    {}
};

struct BleOtaRemovePinReq: public BleOtaMessage
{
    BleOtaRemovePinReq():
        BleOtaMessage(BleOtaHeader::RemovePinReq)
    {}
    static bool isValidSize(size_t size)
    {
        return size == sizeof(BleOtaHeader);
    }
};

struct BleOtaRemovePinResp: public BleOtaMessage
{
    BleOtaRemovePinResp():
        BleOtaMessage(BleOtaHeader::RemovePinResp)
    {}
};