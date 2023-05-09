## Overal
There are possibilities to disable uploads or change pin.

## Disable uploads
Uploads can be disabled at the bagining in `begin()` method or later using `disableUpload()` method.

## Change Pin
Reimplement `BleOtaSecurity` for that and add using `setSecurity()` method.

From protocol side used additional head codes:
```
SET_PIN 0x20
REMOVE_PIN 0x21
```

For example:
```
-> SET_PIN <uint32 pin>
<- OK
-> REMOVE_PIN
<- OK
```
