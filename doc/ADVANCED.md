## Internal buffer
Buffer is storred in RAM (in static memory).
Can be disabled by adding:
```
build_flags =
	-D BLE_OTA_NO_BUFFER
```
It gives more RAM for application, but upload speed will be slower.

## Tuning
Attribute size and buffer size are predefined experimentally. Can be tuned by adding:
```
build_flags =
	-D BLE_OTA_ATTRIBUTE_SIZE=256
	-D BLE_OTA_BUFFER_SIZE=1020
```

## Uploading failure case
When uploading fails then internal buffer usage turns off.
Only after reset it turns on.
