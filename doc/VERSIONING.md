## Overal
Library advertises 5 characteristics in order to indicate device information.
See `ADVERTISING.md`.  
Central can handle this data and check if software should be updated.

## Example
Peripheral device information:
| Field | Value |
| :---- | :---- |
| Manufacture name | My Manufacture |
| Hardware name    | My Hardware    |
| Hardware ver     | `1.0.0`        |
| Software name    | My Software    |
| Software ver     | `1.0.0`        |

Central found new software where:
| Field | Value |
| :---- | :---- |
| Manufacture name | My Manufacture |
| Hardware name    | My Hardware    |
| Hardware ver     | `1.0.0`        |
| Software name    | My Software    |
| Software ver     | `2.0.0`        |

As we see:
- manufacture names are mathed
- hardware names and versions are mathed
- software version in central side is higher

We can update our peripheral device to software version `2.0.0`.
