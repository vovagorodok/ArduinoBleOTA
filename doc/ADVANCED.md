## Internal buffer
Response for each package take a lot of time.
In order to increase speed decided to create buffer in RAM (in static memory).
Each package quickly saves in this buffer.
Once in a while (when buffer is full), all data saves from buffer to flash and response is sent.

Buffer can be disabled by adding:
```
build_flags =
	-D BLE_OTA_NO_BUFFER
```
It gives more RAM for application. But upload speed will be slower because each package saves to flash directly.

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
