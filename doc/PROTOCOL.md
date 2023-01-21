## Overal
Binary protocol where each transfer block contains `uint8` head at the begining.\
Head codes:
```
OK 0x00
NOK 0x01
INCORRECT_FORMAT 0x02
INCORRECT_FIRMWARE_SIZE 0x03
CHECKSUM_ERROR 0x04
INTERNAL_STORAGE_ERROR 0x05

BEGIN 0x10
PACKAGE 0x11
END 0x12
```

## Basic scenario
Designations for examples:\
`->` - recived from central\
`<-` - send to central
```
-> BEGIN <uint32 firmware size>
<- OK <uint32 attribute size> <uint32 buffer size>
-> PACKAGE <uint8[] data>
-> PACKAGE <uint8[] data>
...
-> PACKAGE <uint8[] data>
<- OK
...
-> PACKAGE <uint8[] data>
-> PACKAGE <uint8[] data>
-> END <uint32 crc32 checksum>
<- OK
```
`<uint32 attribute size>` maximal trensfer block.\
`<uint32 buffer size>` internal buffer stored in RAM in order to handle packages without responses.\
Maximal `<uint8[] data>` size is `<uint32 attribute size> - <head size>` bytes, where `<head size>` is 1 byte.\
Internal buffer is created in order to increase upload speed. Packages can be handled immediately, because are stored in RAM instead of flash.
Central should wait response only when buffer is overloaded.\
In order to know more about error codes ckeck scenarios below.

## Comunication lost scenario
In this case buffer usage turns of and uploading starts from begining when `BEGIN` recives again.

## Incorrect transfer block scenarios
Zero length transfer block:
```
-> <zero length transfer block>
<- INCORRECT_FORMAT
```
Incorrect head code:
```
-> <incorrect head code>
<- INCORRECT_FORMAT
```

## Begin failure scenarios
Incorrect firmware size format:
```
-> BEGIN <not uint32 firmware size>
<- INCORRECT_FORMAT
```
Firmware size to large:
```
-> BEGIN
<- INCORRECT_FIRMWARE_SIZE
```
Internal storage error:
```
-> BEGIN
<- INTERNAL_STORAGE_ERROR
```
Transfer start without BEGIN:
```
-> not BEGIN
<- NOK
```

## Package failure scenarios
Uploaded packets size is higher than firmware size:
```
-> PACKAGE
<- INCORRECT_FIRMWARE_SIZE
```

## End failure scenarios
Uploaded packets size is lower than firmware size:
```
-> END
<- INCORRECT_FIRMWARE_SIZE
```
Incorrect checksum size format:
```
-> END <not uint32 crc32 checksum>
<- INCORRECT_FORMAT
```
Checksum doesn't match:
```
-> END
<- CHECKSUM_ERROR
```