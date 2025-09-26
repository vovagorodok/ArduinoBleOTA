#include "BleOtaUploader.h"
#include "BleOtaLogger.h"
#include "ArduinoBleOTA.h"

namespace
{
#define TAG "Upload"

static BleOtaSecurityCallbacks dummySecurityCallbacks{};
static BleOtaUploadCallbacks dummyUploadCallbacks{};
}

BleOtaUploader::BleOtaUploader():
    _state(State::Disable),
    _terminateCode(BleOtaStatus::Ok),
    _storage(),
    _buffer(),
    _decompressor(_storage),
    _checksum(),
    _securityCallbacks(&dummySecurityCallbacks),
    _uploadCallbacks(&dummyUploadCallbacks)
{}

void BleOtaUploader::begin(OTAStorage& storage, bool enable)
{
    _storage.begin(storage);
    _state = enable ? State::Enable : State::Disable;
}

void BleOtaUploader::pull()
{
    if (_state == State::Install)
        handleInstall();
}

void BleOtaUploader::setEnable(bool enable)
{
    BLE_OTA_LOG(TAG, "Enable: %u", enable);

    if (enable)
    {
        _state = State::Enable;
        sendMessage(BleOtaUploadEnableInd{});
    }
    else
    {
        _state = State::Disable;
        sendMessage(BleOtaUploadDisableInd{});
    }
}

void BleOtaUploader::onData(const uint8_t* data, size_t size)
{
    if (_state == State::Install)
    {
        handleError(BleOtaStatus::InstallRunning);
        return;
    }

    if (not BleOtaMessage::isValidSize(size))
    {
        handleError(BleOtaStatus::IncorrectFormat);
        return;
    }

    const auto msg = BleOtaMessage{data};
    BLE_OTA_LOG(TAG, "Receive message: header: %x", msg.header);

    switch (msg.header)
    {
    case BleOtaHeader::InitReq:
        BleOtaInitReq::isValidSize(size) ?
            handleInitReq(BleOtaInitReq{}) :
            handleError(BleOtaStatus::IncorrectFormat);
        break;
    case BleOtaHeader::BeginReq:
        BleOtaBeginReq::isValidSize(size) ?
            handleBeginReq(BleOtaBeginReq{data}) :
            handleError(BleOtaStatus::IncorrectFormat);
        break;
    case BleOtaHeader::PackageReq:
        handlePackageReq(BleOtaPackageReq{data, size});
        break;
    case BleOtaHeader::PackageInd:
        handlePackageInd(BleOtaPackageInd{data, size});
        break;
    case BleOtaHeader::EndReq:
        BleOtaEndReq::isValidSize(size) ?
            handleEndReq(BleOtaEndReq{data}) :
            handleError(BleOtaStatus::IncorrectFormat);
        break;
    case BleOtaHeader::SetPinReq:
        BleOtaSetPinReq::isValidSize(size) ?
            handleSetPinReq(BleOtaSetPinReq{data}) :
            handleError(BleOtaStatus::IncorrectFormat);
        break;
    case BleOtaHeader::RemovePinReq:
        BleOtaRemovePinReq::isValidSize(size) ?
            handleRemovePinReq(BleOtaRemovePinReq{}) :
            handleError(BleOtaStatus::IncorrectFormat);
        break;
    default:
        handleError(BleOtaStatus::IncorrectFormat);
        break;
    }
}

void BleOtaUploader::setSecurityCallbacks(BleOtaSecurityCallbacks& cb)
{
    _securityCallbacks = &cb;
}

void BleOtaUploader::setUploadCallbacks(BleOtaUploadCallbacks& cb)
{
    _uploadCallbacks = &cb;
}

void BleOtaUploader::handleInitReq(const BleOtaInitReq& req)
{
    BLE_OTA_LOG(TAG, "Handle InitReq");

    const bool canCompress = _decompressor.isSupported();
    const bool canChecksum = _checksum.isSupported();
    const bool canUpload = _state != State::Disable;
    const bool canPin = _securityCallbacks != &dummySecurityCallbacks;
    const bool canSignature = false;

    BLE_OTA_LOG(TAG, "Send InitResp: "
        "flags: (compr: %u, checksum: %u, upload: %u, pin: %u, sig: %u)",
        canCompress, canChecksum, canUpload, canPin, canSignature);

    sendMessage(BleOtaInitResp{{
        .compression = canCompress,
        .checksum = canChecksum,
        .upload = canUpload,
        .pin = canPin,
        .signature = canSignature
    }});
}

void BleOtaUploader::handleBeginReq(const BleOtaBeginReq& req)
{
    BLE_OTA_LOG(TAG, "Handle BeginReq: "
        "size: (fw: %lu, pkg: %lu, buff: %lu, compr: %lu), "
        "flags: (compr: %u, checksum: %u)",
        req.firmwareSize, req.packageSize, req.bufferSize, req.compressedSize,
        req.flags.compression, req.flags.checksum);

    if (_state == State::Upload)
    {
        BLE_OTA_LOG(TAG, "Terminate");
        terminateUpload(BleOtaStatus::Ok);
    }

    if (_state == State::Disable)
    {
        handleError(BleOtaStatus::UploadDisabled);
        return;
    }

    const bool withChecksum = req.flags.checksum;
    if (withChecksum)
    {
        if (not _checksum.isSupported())
        {
            handleError(BleOtaStatus::ChecksumNotSupported);
            return;
        }
        _checksum.begin();
    }
    _checksum.setEnable(withChecksum);
    
    const bool withCompression = req.flags.compression;
    if (withCompression)
    {
        if (not _decompressor.isSupported())
        {
            handleError(BleOtaStatus::CompressionNotSupported);
            return;
        }
        _decompressor.begin(req.compressedSize);
    }
    _decompressor.setEnable(withCompression);

    const auto status = _storage.open(req.firmwareSize);
    if (status != BleOtaStatus::Ok)
    {
        handleError(status);
        return;
    }

    _state = State::Upload;

    const uint32_t packageSize = min(req.packageSize, BLE_OTA_PACKAGE_SIZE);
    const uint32_t bufferSize = _buffer.begin(req.bufferSize, req.packageSize);
    BLE_OTA_LOG(TAG, "Send BeginResp: size: (pkg: %lu, buff: %lu)", packageSize, bufferSize);
    sendMessage(BleOtaBeginResp{packageSize, bufferSize});

    _uploadCallbacks->handleUploadBegin();
}

void BleOtaUploader::handlePackageReq(const BleOtaPackageReq& req)
{
    BLE_OTA_LOG(TAG, "Handle PackageReq");

    if (_state != State::Upload)
    {
        handleError(_state == State::Terminate ? _terminateCode : BleOtaStatus::UploadStopped);
        return;
    }

#ifndef BLE_OTA_NO_BUFFER
    BleOtaStatus status;
    if (_buffer.isEnabled())
    {
        status = flushBuffer();
        if (status == BleOtaStatus::Ok)
            status = _buffer.push(req.data, req.size);
    }
    else
    {
        status = push(req.data, req.size);
    }
#else
    const auto status = push(req.data, req.size);
#endif

    if (status != BleOtaStatus::Ok)
    {
        terminateUpload(status);
        handleError(status);
        return;
    }

    sendMessage(BleOtaPackageResp{});
}

void BleOtaUploader::handlePackageInd(const BleOtaPackageInd& ind)
{
    BLE_OTA_LOG(TAG, "Handle PackageInd");

    if (_state != State::Upload)
        return;

#ifndef BLE_OTA_NO_BUFFER
    if (not _buffer.isEnabled())
    {
        terminateUpload(BleOtaStatus::BufferDisabled);
        return;
    }

    const auto status = _buffer.push(ind.data, ind.size);
    if (status != BleOtaStatus::Ok)
    {
        terminateUpload(status);
        return;
    }
#else
    terminateUpload(BleOtaStatus::BufferDisabled);
#endif
}

void BleOtaUploader::handleEndReq(const BleOtaEndReq& req)
{
    BLE_OTA_LOG(TAG, "Handle EndReq: firmware crc: %lu", req.firmwareCrc);

    if (_state != State::Upload)
    {
        handleError(_state == State::Terminate ? _terminateCode : BleOtaStatus::UploadStopped);
        return;
    }

#ifndef BLE_OTA_NO_BUFFER
    if (_buffer.isEnabled())
    {
        const auto status = flushBuffer();
        if (status != BleOtaStatus::Ok)
        {
            terminateUpload(status);
            handleError(status);
            return;
        }
        _buffer.end();
    }
#endif

#ifndef BLE_OTA_NO_COMPRESSION
    if (_decompressor.isEnabled())
    {
        _decompressor.end();
    }
#endif

    if (not _storage.isFull())
    {
        terminateUpload(BleOtaStatus::IncorrectFirmwareSize);
        handleError(BleOtaStatus::IncorrectFirmwareSize);
        return;
    }

#ifndef BLE_OTA_NO_CHECKSUM
    if (_checksum.isEnabled() and _checksum.calc() != req.firmwareCrc)
    {
        terminateUpload(BleOtaStatus::IncorrectChecksum);
        handleError(BleOtaStatus::IncorrectChecksum);
        return;
    }
#endif

    _storage.close();
    _state = State::Install;

    sendMessage(BleOtaEndResp{});

    _uploadCallbacks->handleUploadEnd();
}

void BleOtaUploader::handleSetPinReq(const BleOtaSetPinReq& req)
{
    BLE_OTA_LOG(TAG, "Handle SetPinReq: pin: %lu", req.pin);

    if (_state == State::Upload)
    {
        handleError(BleOtaStatus::UploadRunning);
        return;
    }

    _securityCallbacks->setPinCode(req.pin) ?
        sendMessage(BleOtaSetPinResp{}) :
        sendMessage(BleOtaErrorInd{BleOtaStatus::Nok});
}

void BleOtaUploader::handleRemovePinReq(const BleOtaRemovePinReq& req)
{
    BLE_OTA_LOG(TAG, "Handle RemovePinReq");

    if (_state == State::Upload)
    {
        handleError(BleOtaStatus::UploadRunning);
        return;
    }

    _securityCallbacks->removePinCode() ?
        sendMessage(BleOtaRemovePinResp{}) :
        sendMessage(BleOtaErrorInd{BleOtaStatus::Nok});
}

void BleOtaUploader::handleInstall()
{
    delay(250);
    _storage.apply();
    while (true);
}

void BleOtaUploader::handleError(BleOtaStatus code)
{
    sendMessage(BleOtaErrorInd{code});
    _uploadCallbacks->handleUploadError(code);
}

void BleOtaUploader::send(const uint8_t* data, size_t size)
{
    ArduinoBleOTA.send(data, size);
}

template <typename T>
void BleOtaUploader::sendMessage(const T& msg)
{
    BLE_OTA_LOG(TAG, "Send message: header: %x", msg.header);
    send(reinterpret_cast<const uint8_t*>(&msg), sizeof(T));
}

void BleOtaUploader::terminateUpload(BleOtaStatus code)
{
    _storage.clear();
    _storage.close();
    _state = State::Terminate;
    _terminateCode = code;
#ifndef BLE_OTA_NO_BUFFER
    if (_buffer.isEnabled())
    {
        _buffer.setEnable(false);
        _buffer.end();
    }
#endif
#ifndef BLE_OTA_NO_COMPRESSION
    if (_decompressor.isEnabled())
    {
        _decompressor.end();
    }
#endif
}

BleOtaStatus BleOtaUploader::push(const uint8_t* data, size_t size)
{
#ifndef BLE_OTA_NO_CHECKSUM
    if (_checksum.isEnabled())
        _checksum.push(data, size);
#endif

#ifndef BLE_OTA_NO_COMPRESSION
    return _decompressor.isEnabled() ?
        _decompressor.push(data, size) :
        _storage.push(data, size);
#else
    return _storage.push(data, size);
#endif
}

BleOtaStatus BleOtaUploader::flushBuffer()
{
#ifndef BLE_OTA_NO_BUFFER
    const auto status = push(_buffer.data(), _buffer.size());
    _buffer.clear();
    return status;
#else
    return BleOtaStatus::BufferDisabled;
#endif
}