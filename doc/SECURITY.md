## Overal
There are possibilities:
- disable upload
- sign the software
- change pin code

## Messages
### UploadEnableInd
Peripheral enables upload
| Field | Type | Info |
| :---- | :--- | :--- |
| Header | `u8` | `0x11` |

### UploadDisableInd
Peripheral disables upload
| Field | Type | Info |
| :---- | :--- | :--- |
| Header | `u8` | `0x12` |

### SignatureReq
Central delivers signature data
| Field | Type | Info |
| :---- | :--- | :--- |
| Header | `u8` | `0x20` |
| Data | `u8[]` | Signature data |

### SignatureResp
| Field | Type | Info |
| :---- | :--- | :--- |
| Header | `u8` | `0x21` |

### SetPinReq
Central sets pin
| Field | Type | Info |
| :---- | :--- | :--- |
| Header | `u8` | `0x30` |
| Pin | `u32` | Pin should contain exact 6 digits |

### SetPinResp
| Field | Type | Info |
| :---- | :--- | :--- |
| Header | `u8` | `0x31` |

### RemovePinReq
Central removes pin
| Field | Type | Info |
| :---- | :--- | :--- |
| Header | `u8` | `0x32` |

### RemovePinResp
| Field | Type | Info |
| :---- | :--- | :--- |
| Header | `u8` | `0x33` |

## Disable upload
Upload can be disabled at the bagining in `begin()` method or later by `setUploadEnable(false)` method.

## Signature
Generate private key (keep this secret)
```
openssl genrsa -out priv_key.pem 2048
```
Generate public key
```
openssl rsa -in priv_key.pem -pubout > rsa_key.pub
```
Generate signature file if needed by using SHA256 and private key
```
openssl dgst -sign priv_key.pem -keyform PEM -sha256 -out signature.sig -binary firmware.bin
```
Create firmware file with signatue if needed
```
cat firmware.bin signature.sig > firmware.sig.bin
```
Enable signature checking by copy public key to source code and by `setSignatureKey()` method.

## Change pin code
Enable pin support by reimplement `BleOtaPinCallbacks` and by `setPinCallbacks()` method.
