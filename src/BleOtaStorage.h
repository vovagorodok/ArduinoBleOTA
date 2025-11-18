#pragma once
#include "BleOtaStatus.h"
#include "BleOtaStorageDefines.h"
#include "BleOtaPercentages.h"

class BleOtaStorage
{
public:
    BleOtaStorage();

    void begin(OTAStorage& storage);
    BleOtaStatus open(size_t firmwareSize);
    BleOtaStatus push(const uint8_t* data, size_t size);
    void close();
    void apply();
    void clear();
    bool isFull() const;
    BleOtaPercentages calcProgress() const;

private:
    OTAStorage* _storage;
    size_t _firmwareSize;
    size_t _size;
};