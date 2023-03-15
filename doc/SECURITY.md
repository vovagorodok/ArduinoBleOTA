## Overal
There are possibilities to disable uploads or change pin.

## Change Pin
Can be used additional head codes:
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
