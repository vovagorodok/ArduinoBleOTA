#include "BleOtaInternalStorageESP.h"
#ifdef BLE_OTA_STORAGE_LIB_INTERNAL
#include "BleOtaDefinesArduino.h"
#ifdef ARDUINO_ARCH_ESP32
#include <Update.h>
#endif

bool OTAStorage::open(int size)
{
  return Update.begin(size);
}

size_t OTAStorage::push(const uint8_t* data, size_t size)
{
  return Update.write(const_cast<uint8_t*>(data), size);
}

void OTAStorage::close()
{
  Update.end();
}

void OTAStorage::clear()
{
#ifdef ARDUINO_ARCH_ESP32
  Update.abort();
#endif
}

void OTAStorage::apply()
{
  ESP.restart();
}

size_t OTAStorage::maxSize() const
{
  return ESP.getFlashChipSize() / 2; // Update.begin() in open() does the exact check
}

OTAStorage InternalStorage;
#endif