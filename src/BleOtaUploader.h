#pragma once
#include "BleOtaSizes.h"
#include "BleOtaMessages.h"
#include "BleOtaStorage.h"
#include "BleOtaBuffer.h"
#include "BleOtaDecompressor.h"
#include "BleOtaChecksum.h"
#include "BleOtaSignature.h"
#include "BleOtaPinCallbacks.h"
#include "BleOtaUploadCallbacks.h"

class BleOtaUploader
{
public:
    BleOtaUploader();

    void begin(OTAStorage& storage, bool enable);
    void pull();
    void setEnable(bool enable);
    void onData(const uint8_t* data, size_t size);

    bool setSignatureKey(const char* key, size_t size);
    void setPinCallbacks(BleOtaPinCallbacks&);
    void setUploadCallbacks(BleOtaUploadCallbacks&);

private:
    void handleInitReq(const BleOtaInitReq& req);
    void handleBeginReq(const BleOtaBeginReq& req);
    void handlePackageReq(const BleOtaPackageReq& req);
    void handlePackageInd(const BleOtaPackageInd& ind);
    void handleEndReq(const BleOtaEndReq& req);
    void handleSetPinReq(const BleOtaSetPinReq& req);
    void handleRemovePinReq(const BleOtaRemovePinReq& req);
    void handleSignatureReq(const BleOtaSignatureReq& req);
    void handleInstall();
    void handleError(BleOtaStatus code);
    void send(const uint8_t* data, size_t size);
    template <typename T>
    void sendMessage(const T& msg);
    void terminateUpload(BleOtaStatus code);
    BleOtaStatus push(const uint8_t* data, size_t size);
    BleOtaStatus flushBuffer();

    enum class State {
        Disable,
        Enable,
        Upload,
        Install,
        Terminate
    } _state;
    BleOtaStatus _terminateCode;

    BleOtaStorage _storage;
    BleOtaBuffer _buffer;
    BleOtaDecompressor _decompressor;
    BleOtaChecksum _checksum;
    BleOtaSignature _signature;

    BleOtaPinCallbacks* _pinCallbacks;
    BleOtaUploadCallbacks* _uploadCallbacks;
};