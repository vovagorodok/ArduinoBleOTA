# Arduino BLE OTA
Arduino Bluetooth LE Over The Air.
Simple library for upload firmware over Bluetooth.
Has built in checksum/integrity protection and
software/hardware name/version indication.

More about library in `doc` folder.

## Upload possibilities
For upload using scripts see `tools` folder.  
For upload using mobile/web/desktop application see:  
https://github.com/vovagorodok/ble_ota_app

## Configuration
By default `ArduinoBLE` is used. For `NimBLE-Arduino` some changes are reguired.  
For PlatformIO. Add to `platformio.ini`:
```
build_flags = 
	-D USE_NIM_BLE_ARDUINO_LIB
```

For Arduino IDE. At boards package installation folder create `platform.local.txt`:
```
compiler.cpp.extra_flags=-D USE_NIM_BLE_ARDUINO_LIB
```

For using the native ESP32 BLE library, use the `USE_NATIVE_ESP32_BLE_LIB` flag instad of `USE_NIM_BLE_ARDUINO_LIB`.

## Opened discussions
https://github.com/JAndrassy/ArduinoOTA/discussions/157  
https://github.com/fbiego/ESP32_BLE_OTA_Arduino/issues/19
