## Overal
Binary protocol where each transfer block contains `u8` head at the begining.  

## Messages
### InitReq
Central checks peripheral compatibilities
<table>
  <tr>
    <th style="width: 140px;">Field</th>
    <th style="width: 20px;">Size</th>
    <th style="width: 350px;">Info</th>
  </tr>
  <tr>
    <td>Head</td>
    <td><code>u8</code></td>
    <td><code>0x01</code></td>
  </tr>
</table>

### InitResp
<table>
  <tr>
    <th style="width: 140px;">Field</th>
    <th style="width: 20px;">Size</th>
    <th style="width: 350px;">Info</th>
  </tr>
  <tr>
    <td>Head</td>
    <td><code>u8</code></td>
    <td><code>0x02</code></td>
  </tr>
  <tr>
    <td>Flags</td>
    <td><code>u8</code></td>
    <td>Described below</td>
  </tr>
</table>
<table>
  <tr>
    <th style="width: 140px;">Flag</th>
    <th style="width: 250px;">Info</th>
  </tr>
  <tr>
    <td>Compression</td>
    <td>Compression supported</td>
  </tr>
  <tr>
    <td>Checksum</td>
    <td>Checksum supported</td>
  </tr>
  <tr>
    <td>Upload</td>
    <td>Upload enabled</td>
  </tr>
  <tr>
    <td>Pin</td>
    <td>Pin change supported</td>
  </tr>
  <tr>
    <td>Signature</td>
    <td>Signature required</td>
  </tr>
</table>

### BeginReq
Central begins uploading
<table>
  <tr>
    <th style="width: 140px;">Field</th>
    <th style="width: 20px;">Size</th>
    <th style="width: 350px;">Info</th>
  </tr>
  <tr>
    <td>Head</td>
    <td><code>u8</code></td>
    <td><code>0x03</code></td>
  </tr>
  <tr>
    <td>Firmware size</td>
    <td><code>u32</code></td>
    <td>Binary file size</td>
  </tr>
  <tr>
    <td>Package size</td>
    <td><code>u32</code></td>
    <td>Max package size, <code>0xFFFFFFFF</code> if any</td>
  </tr>
  <tr>
    <td>Buffer size</td>
    <td><code>u32</code></td>
    <td>Internal buffer, <code>0xFFFFFFFF</code> if any, <code>0</code> if disable</td>
  </tr>
  <tr>
    <td>Compressed size</td>
    <td><code>u32</code></td>
    <td>Binary file size after compression</td>
  </tr>
  <tr>
    <td>Flags</td>
    <td><code>u8</code></td>
    <td>Described below</td>
  </tr>
</table>
<table>
  <tr>
    <th style="width: 140px;">Flag</th>
    <th style="width: 250px;">Info</th>
  </tr>
  <tr>
    <td>Compression</td>
    <td>Binary file compressed</td>
  </tr>
  <tr>
    <td>Checksum</td>
    <td>Checksum calculation required</td>
  </tr>
</table>

### BeginResp
<table>
  <tr>
    <th style="width: 140px;">Field</th>
    <th style="width: 20px;">Size</th>
    <th style="width: 350px;">Info</th>
  </tr>
  <tr>
    <td>Head</td>
    <td><code>u8</code></td>
    <td><code>0x04</code></td>
  </tr>
  <tr>
    <td>Package size</td>
    <td><code>u32</code></td>
    <td>Negotiated max package size</td>
  </tr>
  <tr>
    <td>Buffer size</td>
    <td><code>u32</code></td>
    <td>Negotiated internal buffer, <code>0</code> if disable</td>
  </tr>
</table>

### PackageInd
Central delivers binary data that will be stored in buffer, response not needed
<table>
  <tr>
    <th style="width: 140px;">Field</th>
    <th style="width: 20px;">Size</th>
    <th style="width: 350px;">Info</th>
  </tr>
  <tr>
    <td>Head</td>
    <td><code>u8</code></td>
    <td><code>0x05</code></td>
  </tr>
  <tr>
    <td>Data</td>
    <td><code>u8[]</code></td>
    <td>Binary data</td>
  </tr>
</table>

### PackageReq
Central delivers binary data that will be stored with buffered data in flash, response required
<table>
  <tr>
    <th style="width: 140px;">Field</th>
    <th style="width: 20px;">Size</th>
    <th style="width: 350px;">Info</th>
  </tr>
  <tr>
    <td>Head</td>
    <td><code>u8</code></td>
    <td><code>0x06</code></td>
  </tr>
  <tr>
    <td>Data</td>
    <td><code>u8[]</code></td>
    <td>Binary data</td>
  </tr>
</table>

### PackageResp
<table>
  <tr>
    <th style="width: 140px;">Field</th>
    <th style="width: 20px;">Size</th>
    <th style="width: 350px;">Info</th>
  </tr>
  <tr>
    <td>Head</td>
    <td><code>u8</code></td>
    <td><code>0x07</code></td>
  </tr>
</table>

### EndReq
Central ends uploading
<table>
  <tr>
    <th style="width: 140px;">Field</th>
    <th style="width: 20px;">Size</th>
    <th style="width: 350px;">Info</th>
  </tr>
  <tr>
    <td>Head</td>
    <td><code>u8</code></td>
    <td><code>0x08</code></td>
  </tr>
  <tr>
    <td>Firmware checksum</td>
    <td><code>u32</code></td>
    <td>Calulated checksum, <code>0</code> when disabled</td>
  </tr>
</table>

### EndResp
<table>
  <tr>
    <th style="width: 140px;">Field</th>
    <th style="width: 20px;">Size</th>
    <th style="width: 350px;">Info</th>
  </tr>
  <tr>
    <td>Head</td>
    <td><code>u8</code></td>
    <td><code>0x09</code></td>
  </tr>
</table>

### ErrorInd
Peripheral indicates error
<table>
  <tr>
    <th style="width: 140px;">Field</th>
    <th style="width: 20px;">Size</th>
    <th style="width: 350px;">Info</th>
  </tr>
  <tr>
    <td>Head</td>
    <td><code>u8</code></td>
    <td><code>0x10</code></td>
  </tr>
  <tr>
    <td>Code</td>
    <td><code>u8</code></td>
    <td>Described below</td>
  </tr>
</table>

| Code | Info |
| ---- | ---- |
| Ok | `0x00` |
| Nok | `0x01` |
| Incorrect format | `0x02` |
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
| Incorrect signature | `0x50` |

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

## Comunication lost scenario
In this case buffer usage turns of and uploading starts from begining when `BeginReq` recives again.

## Incorrect transfer block scenarios
Zero size transfer block:
```
c) <Zero size transfer block>
p) ErrorInd: Incorrect format
```
Incorrect head code:
```
c) <Incorrect head code>
p) ErrorInd: Incorrect format
```

## Begin failure scenarios
Incorrect firmware size format:
```
c) BeginReq <Incorrect message size>
p) ErrorInd: Incorrect format
```
Firmware size to large:
```
c) BeginReq
p) ErrorInd: Incorrect firmware size
```
Internal storage error:
```
c) BeginReq
p) ErrorInd: Internal storage error
```
Upload disabled:
```
c) BeginReq
p) ErrorInd: Upload disabled
```
Transfer start without BeginReq:
```
c) PackageReq
p) ErrorInd: Upload stopped
```

## Package failure scenarios
Uploaded packets size is higher than firmware size:
```
c) PackageReq
p) ErrorInd: Incorrect firmware size
```

## End failure scenarios
Uploaded packets size is lower than firmware size:
```
c) EndReq
p) ErrorInd: Incorrect firmware size
```
Incorrect checksum size format:
```
c) EndReq <Incorrect message size>
p) ErrorInd: Incorrect format
```
Checksum doesn't match:
```
c) EndReq
p) ErrorInd: Incorrect checksum 
```