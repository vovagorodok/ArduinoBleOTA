#include "BleOtaInternalStorageFake.h"
#ifdef BLE_OTA_STORAGE_LIB_FAKE
#include "BleOtaDefinesArduino.h"

bool OTAStorage::open(int size)
{
  return false;
}

size_t OTAStorage::push(const uint8_t* data, size_t size)
{
  return 0;
}

void OTAStorage::close()
{}

void OTAStorage::clear()
{}

void OTAStorage::apply()
{}

size_t OTAStorage::maxSize() const
{
  return 0;
}

OTAStorage InternalStorage;
#endif