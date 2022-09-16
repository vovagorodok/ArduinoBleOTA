from bluezero import adapter
from bluezero import central

OTA_SERVICE_UUID = "c68680a2-d922-11ec-bd40-7ff604147105"
OTA_CHARACTERISTIC_UUID_RX = "c6868174-d922-11ec-bd41-c71bb0ce905a"
OTA_CHARACTERISTIC_UUID_TX = "c6868246-d922-11ec-bd42-7b10244d223f"

class Updater:
    def __init__(self):
        self.rx_char = None
        self.tx_char = None

    def scan_ota_devices(self, adapter_address=None, timeout=5.0):
        for dongle in adapter.Adapter.available():
            if adapter_address and adapter_address.upper() != dongle.address():
                continue

            dongle.nearby_discovery(timeout=timeout)

            for dev in central.Central.available(dongle.address):
                if OTA_SERVICE_UUID.lower() in dev.uuids:
                    yield dev

    def connect_and_run(self, dev):
        monitor = central.Central(adapter_addr=dev.adapter, device_addr=dev.address)
        self.rx_char = monitor.add_characteristic(OTA_SERVICE_UUID, OTA_CHARACTERISTIC_UUID_RX)
        self.tx_char = monitor.add_characteristic(OTA_SERVICE_UUID, OTA_CHARACTERISTIC_UUID_TX)

        print("Connecting to " + dev.alias)
        monitor.connect()
        if not monitor.connected:
            print("Didn't connect to device!")
            return

        self.tx_char.start_notify()
        self.tx_char.add_characteristic_cb(self.on_write)
        self.rx_char.value = 0x00

        try:
            monitor.run()
        except KeyboardInterrupt:
            print("Disconnecting")

        self.tx_char.stop_notify()
        monitor.disconnect()

    def on_write(self, iface, changed_props, invalidated_props):
        value = self.tx_char.value
        if not value:
            return

        print("Recived: " + value)

if __name__ == '__main__':
    updater = Updater()
    devices = updater.scan_ota_devices()
    for device in devices:
        print("OTA Device Found!")
        updater.connect_and_run(device)
        break