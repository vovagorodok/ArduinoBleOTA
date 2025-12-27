#ifndef ARDUINO
#include <ArduinoBleOTA.h>

int main() {
  ArduinoBleOTA.begin("ArduinoBleOTA", InternalStorage);
  return 0;
}
#endif