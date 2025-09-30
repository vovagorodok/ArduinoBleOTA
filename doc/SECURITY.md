## Overal
There are possibilities:
- disable upload
- change pin code
- sign the software

## Messages
### UploadEnableInd
Peripheral enables upload
| Field | Type | Info |
| :---- | :--- | :--- |
| Head | `u8` | `0x11` |

### UploadDisableInd
Peripheral disables upload
| Field | Type | Info |
| :---- | :--- | :--- |
| Head | `u8` | `0x12` |

### SetPinReq
Central sets pin
| Field | Type | Info |
| :---- | :--- | :--- |
| Head | `u8` | `0x20` |
| Pin | `u32` | Pin should contain exact 6 digits |

### SetPinResp
| Field | Type | Info |
| :---- | :--- | :--- |
| Head | `u8` | `0x21` |

### RemovePinReq
Central removes pin
| Field | Type | Info |
| :---- | :--- | :--- |
| Head | `u8` | `0x22` |

### RemovePinResp
| Field | Type | Info |
| :---- | :--- | :--- |
| Head | `u8` | `0x23` |

### SignatureReq
Central delivers signature data
| Field | Type | Info |
| :---- | :--- | :--- |
| Head | `u8` | `0x30` |
| Data | `u8[]` | Signature data |

### SignatureResp
| Field | Type | Info |
| :---- | :--- | :--- |
| Head | `u8` | `0x31` |

## Disable upload
Uploads can be disabled at the bagining in `begin()` method or later by `setUploadEnable(false)` method.

## Change pin code
Enable pin support by reimplement `BleOtaPinCallbacks` and by `setPinCallbacks()` method.

## Signature
Generate private key (keep this secret)
```
openssl genrsa -out priv_key.pem 2048
```
Generate public key
```
openssl rsa -in priv_key.pem -pubout > rsa_key.pub
```
Signature can be generated using SHA256 and private key
```
openssl dgst -sign priv_key.pem -keyform PEM -sha256 -out signature.sig -binary firmware.bin
```
Enable signature checking by copy public key to source code and by `setSignatureKey()` method.
