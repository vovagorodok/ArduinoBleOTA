## Overal
There are possibilities to disable uploads or change pin code.

## Messages
### UploadEnableInd
| Field | Size | Info|
| ---- | ---- | ---- |
| Head | `u8` | `0x11` |

### UploadDisableInd
| Field | Size | Info|
| ---- | ---- | ---- |
| Head | `u8` | `0x12` |

### SetPinReq
| Field | Size | Info|
| ---- | ---- | ---- |
| Head | `u8` | `0x20` |
| Pin | `u32` | Pin should contain exact 6 digits |

### SetPinResp
| Field | Size | Info|
| ---- | ---- | ---- |
| Head | `u8` | `0x21` |

### RemovePinReq
| Field | Size | Info|
| ---- | ---- | ---- |
| Head | `u8` | `0x22` |

### RemovePinResp
| Field | Size | Info|
| ---- | ---- | ---- |
| Head | `u8` | `0x23` |

## Disable uploads
Uploads can be disabled at the bagining in `begin()` method or later using `setEnableUpload(false)` method.

## Change pin code
Reimplement `BleOtaSecurityCallbacks` for that and add by `setSecurityCallbacks()` method.