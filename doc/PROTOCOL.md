## Overal
Binary protocol with little-endian byte order and LSB-first bit order.  
Each message contains `u8` header at the begining.  
Additionally require zlib for compression and sha256 for signature if supported.

## Messages
### InitReq
Central checks peripheral compatibilities
| Field | Type | Info |
| :---- | :--- | :--- |
| Header | `u8` | `0x01` |

### InitResp
| Field | Type | Info |
| :---- | :--- | :--- |
| Header | `u8` | `0x02` |
| Flags | `u8` | Described below |

| Flag | Info |
| :--- | :--- |
| Compression | Compression supported |
| Checksum | Checksum supported |
| Upload | Upload enabled |
| Signature | Signature required |
| Pin | Pin change supported |

### BeginReq
Central begins uploading
| Field | Type | Info |
| :---- | :--- | :--- |
| Header | `u8` | `0x03` |
| Firmware size | `u32` | Binary file size |
| Package size | `u32` | Max package size, `0xFFFFFFFF` if any |
| Buffer size | `u32` | Internal buffer size, `0xFFFFFFFF` if any, `0` if disable |
| Compressed size | `u32` | Binary file size after compression, any value if disable |
| Flags | `u8` | Described below |

| Flag | Info |
| :--- | :--- |
| Compression | Binary file compressed |
| Checksum | Checksum calculation required |

### BeginResp
| Field | Type | Info |
| :---- | :--- | :--- |
| Header | `u8` | `0x04` |
| Package size | `u32` | Negotiated max package size |
| Buffer size | `u32` | Negotiated internal buffer size, `0` if disable |

### PackageInd
Central delivers binary data that will be stored in buffer, response not needed
| Field | Type | Info |
| :---- | :--- | :--- |
| Header | `u8` | `0x05` |
| Data | `u8[]` | Binary data |

### PackageReq
Central delivers binary data that will be stored in flash or flush buffer if enable, response required
| Field | Type | Info |
| :---- | :--- | :--- |
| Header | `u8` | `0x06` |
| Data | `u8[]` | Binary data |

### PackageResp
| Field | Type | Info |
| :---- | :--- | :--- |
| Header | `u8` | `0x07` |

### EndReq
Central ends uploading
| Field | Type | Info |
| :---- | :--- | :--- |
| Header | `u8` | `0x08` |
| Firmware checksum | `u32` | Calulated checksum, any value if disable |

### EndResp
| Field | Type | Info |
| :---- | :--- | :--- |
| Header | `u8` | `0x09` |

### ErrorInd
Peripheral indicates error
| Field | Type | Info |
| :---- | :--- | :--- |
| Header | `u8` | `0x10` |
| Code | `u8` | Described below |

| Code | Info |
| :--- | :--- |
| Ok | `0x00` |
| Incorrect message size | `0x01` |
| Incorrect message header | `0x02` |
| Incorrect firmware size | `0x03` |
| Internal storage error | `0x04` |
| Upload disabled | `0x10` |
| Upload running | `0x11` |
| Upload stopped | `0x12` |
| Install running | `0x13` |
| Buffer disabled | `0x20` |
| Buffer overflow | `0x21` |
| Compression not supported | `0x30` |
| Incorrect compression | `0x31` |
| Incorrect compressed size | `0x32` |
| Incorrect compression checksum | `0x33` |
| Incorrect compression param | `0x34` |
| Incorrect compression end | `0x35` |
| Checksum not supported | `0x40` |
| Incorrect checksum | `0x41` |
| Signature not supported | `0x50` |
| Incorrect signature | `0x51` |
| Incorrect signature size | `0x52` |
| Pin not supported | `0x60` |
| Pin change error | `0x61` |

## Basic scenario
Designations for examples:  
`c)` - central send  
`p)` - peripheral send
```
c) InitReq
p) InitResp
...
c) BeginReq
p) BeginResp
c) PackageInd
..
c) PackageReq
p) PackageResp
...
c) PackageInd
c) EndReq
p) EndResp
```

Internal buffer is created in order to increase upload speed. Packages can be handled immediately, because are stored in RAM instead of flash.
Central send `PackageReq` when buffer is overloaded.  
In order to know more about error codes ckeck scenarios below.

## Comunication lost
In this case buffer usage turns of and uploading starts from begining when `BeginReq` recives again.

## Incorrect message
Incorrect message size
```
c) <Incorrect message size>
p) ErrorInd: Incorrect message size
```
Incorrect header code
```
c) <Incorrect header code>
p) ErrorInd: Incorrect message header
```

## Incorrect messages order
Upload not started
```
c) PackageReq
p) ErrorInd: Upload stopped
```
Install running
```
c) PackageReq
p) ErrorInd: Install running
```

## Begin failure
Firmware size to large
```
c) BeginReq
p) ErrorInd: Incorrect firmware size
```
Internal storage error
```
c) BeginReq
p) ErrorInd: Internal storage error
```
Upload disabled
```
c) BeginReq
p) ErrorInd: Upload disabled
```

## Package failure
Uploaded packets size is higher than firmware size
```
c) PackageReq
p) ErrorInd: Incorrect firmware size
```

## End failure
Uploaded packets size is lower than firmware size
```
c) EndReq
p) ErrorInd: Incorrect firmware size
```
Checksum doesn't match
```
c) EndReq
p) ErrorInd: Incorrect checksum
```