# Arduino BLE OTA
Arduino Bluetooth LE Over The Air.
Simple library for upload firmware over Bluetooth.
Has build in checksum integrity protection and
software/hardware type/version protection.

## Upload by python script
```
python3 tools/uploader.py .pio/build/esp32dev/firmware.bin
```

## Documentation
Technical information at `DOC.md`.\
More about protocol at `PROTOCOL.md`.

## Opened discussions
https://github.com/JAndrassy/ArduinoOTA/discussions/157 \
https://github.com/fbiego/ESP32_BLE_OTA_Arduino/issues/19