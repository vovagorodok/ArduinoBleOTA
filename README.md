# Arduino BLE OTA
Arduino Bluetooth LE Over The Air.
Simple library for upload firmware over Bluetooth.
Has built in checksum/integrity protection and
software/hardware name/version indication.

By default `ArduinoBLE` is used.
If `NimBLE-Arduino` is used in your project, add to platformio.ini:
```
build_flags = 
	-D USE_NIM_BLE_ARDUINO_LIB
```
More about library in `doc` folder.

## Upload possibilities
For upload using scripts see `tools` folder.\
For upload using mobile/web/desktop application see:\
https://github.com/vovagorodok/ble_ota_app

## Opened discussions
https://github.com/JAndrassy/ArduinoOTA/discussions/157 \
https://github.com/fbiego/ESP32_BLE_OTA_Arduino/issues/19
