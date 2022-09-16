from bluezero import adapter
from bluezero import central

OTA_SERVICE_UUID = "c68680a2-d922-11ec-bd40-7ff604147105"
OTA_CHARACTERISTIC_UUID_RX = "c6868174-d922-11ec-bd41-c71bb0ce905a"
OTA_CHARACTERISTIC_UUID_TX = "c6868246-d922-11ec-bd42-7b10244d223f"

def scan_ota_devices(adapter_address=None, timeout=5.0):
    for dongle in adapter.Adapter.available():
        if adapter_address and adapter_address.upper() != dongle.address():
            continue

        dongle.nearby_discovery(timeout=timeout)

        for dev in central.Central.available(dongle.address):
            if OTA_SERVICE_UUID.lower() in dev.uuids:
                yield dev

if __name__ == '__main__':
    devices = scan_ota_devices()
    for device in devices:
        print("OTA Device Found!")
        break