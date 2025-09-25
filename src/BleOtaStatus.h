#pragma once
#include <Arduino.h>

enum BleOtaStatus: uint8_t
{
    Ok = 0x00,
    Nok = 0x01,
    IncorrectFormat = 0x02,
    IncorrectFirmwareSize = 0x03,
    InternalStorageError = 0x04,

    UploadDisabled = 0x10,
    UploadRunning = 0x11,
    UploadStopped = 0x12,
    InstallRunning = 0x13,

    BufferDisabled = 0x20,
    BufferOverflow = 0x21,

    CompressionNotSupported = 0x30,
    IncorrectCompression = 0x31,
    IncorrectCompressedSize = 0x32,
    IncorrectCompressionChecksum = 0x33,
    IncorrectCompressionParam = 0x34,
    IncorrectCompressionEnd = 0x35,

    ChecksumNotSupported = 0x40,
    IncorrectChecksum = 0x41,

    IncorrectSignature = 0x50,
};