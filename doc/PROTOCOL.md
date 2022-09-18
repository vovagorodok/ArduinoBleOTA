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
<- OK
-> PACKAGE <uint8[] data>
<- OK
...
-> PACKAGE <uint8[] data>
<- OK
-> END <uint32 crc32 checksum>
<- OK
```
Maximal `<uint8[] data>` size 511 bytes.

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
