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

def connect_and_run(dev):
    monitor = central.Central(adapter_addr=dev.adapter, device_addr=dev.address)
    rx_char = monitor.add_characteristic(OTA_SERVICE_UUID, OTA_CHARACTERISTIC_UUID_RX)
    tx_char = monitor.add_characteristic(OTA_SERVICE_UUID, OTA_CHARACTERISTIC_UUID_TX)

    print("Connecting to " + dev.alias)
    monitor.connect()
    if not monitor.connected:
        print("Didn't connect to device!")
        return

    tx_char.start_notify()
    tx_char.add_characteristic_cb(on_write)
    rx_char.value = 0x00

    try:
        monitor.run()
    except KeyboardInterrupt:
        print("Disconnecting")

    tx_char.stop_notify()
    monitor.disconnect()


def on_write(iface, changed_props, invalidated_props):
    value = changed_props.get('Value', None)
    if not value:
        return

    print("Recived: " + value)

if __name__ == '__main__':
    devices = scan_ota_devices()
    for device in devices:
        print("OTA Device Found!")
        connect_and_run(device)
        break