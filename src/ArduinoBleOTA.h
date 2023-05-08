#pragma once


#ifdef ESP32
    #ifdef USE_NIM_BLE_ARDUINO_LIB  
        #include "ArduinoBleOtaClassNimBle.h"
    #else
#include "ArduinoBleOtaClassESP32Ble.h"
#endif
#else
#include "ArduinoBleOtaClass.h"
#endif