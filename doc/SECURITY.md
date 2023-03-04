## Overal
Security uses same protocol. Additional head codes are used:
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
