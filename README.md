# Arduino BLE OTA
Arduino Bluetooth LE Over The Air.
Simple library for upload firmware over Bluetooth.
Has built in checksum integrity protection and
software/hardware type/version indication.

## Upload by python script
```
python3 tools/uploader.py .pio/build/esp32dev/firmware.bin
```

## Upload by mobile/web/desktop application
https://github.com/vovagorodok/ble_ota_app

## Documentation
Advanced usage at `ADVANCED.md`.\
More about uuids advertising at `ADVERTISING.md`.\
More about protocol at `PROTOCOL.md`.

## Opened discussions
https://github.com/JAndrassy/ArduinoOTA/discussions/157 \
https://github.com/fbiego/ESP32_BLE_OTA_Arduino/issues/19
