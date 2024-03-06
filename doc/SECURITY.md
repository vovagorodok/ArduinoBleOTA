## Overal
There are possibilities to disable uploads or change pin code.

## Disable uploads
Uploads can be disabled at the bagining in `begin()` method or later using `disableUpload()` method.

## Change Pin Code
Reimplement `BleOtaSecurityCallbacks` for that and add by `setSecurityCallbacks()` method.

From protocol side used additional head codes:
```
SET_PIN_CODE 0x20
REMOVE_PIN_CODE 0x21
```

For example:
```
-> SET_PIN_CODE <uint32 pin code>
<- OK
-> REMOVE_PIN_CODE
<- OK
```
