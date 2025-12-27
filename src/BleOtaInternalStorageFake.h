#pragma once
#include "BleOtaStorageDefines.h"
#ifdef BLE_OTA_STORAGE_LIB_FAKE
#include "BleOtaDefinesArduino.h"

class OTAStorage
{
public:
  bool open(int size);
  size_t push(const uint8_t* data, size_t size);
  void close();
  void clear();
  void apply();
  size_t maxSize() const;
};

extern OTAStorage InternalStorage;
#endif