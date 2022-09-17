from bluezero import adapter
from bluezero import central
import sys
import zlib
import os
import datetime

OTA_SERVICE_UUID = "c68680a2-d922-11ec-bd40-7ff604147105"
OTA_CHARACTERISTIC_UUID_RX = "c6868174-d922-11ec-bd41-c71bb0ce905a"
OTA_CHARACTERISTIC_UUID_TX = "c6868246-d922-11ec-bd42-7b10244d223f"
MTU_SIZE = 512
HEADER_SZIE = 1
BUFFER_SIZE = MTU_SIZE - HEADER_SZIE

OK = 0x00
NOK = 0x01
INCORRECT_FORMAT = 0x02
INCORRECT_FIRMWARE_SIZE = 0x03
CHECKSUM_ERROR = 0x04
INTERNAL_STORAGE_ERROR = 0x05

BEGIN = 0x10
PACKAGE = 0x11
END = 0x12
INSTALL = 0x13

respToStr = {
    NOK: "Not ok",
    INCORRECT_FORMAT: "Incorrect format",
    INCORRECT_FIRMWARE_SIZE: "Incorrect firmware size",
    CHECKSUM_ERROR: "Checksum error",
    INTERNAL_STORAGE_ERROR: "Internal storage error"
}

def file_size(path):
    if os.path.isfile(path):
        file_info = os.stat(path)
        return file_info.st_size

def u8_bytes_to_int(value):
    return int.from_bytes(value, 'little', signed=False)

def int_to_u8_bytes(value):
    return list(int.to_bytes(value, 1, 'little', signed=False))

def int_to_u32_bytes(value):
    return list(int.to_bytes(value, 4, 'little', signed=False))

def scan_ota_devices(adapter_address=None, timeout=5.0):
    for dongle in adapter.Adapter.available():
        if adapter_address and adapter_address.upper() != dongle.address():
            continue

        dongle.nearby_discovery(timeout=timeout)

        for dev in central.Central.available(dongle.address):
            if OTA_SERVICE_UUID.lower() in dev.uuids:
                yield dev

def handleResponse(resp):
    if (resp == OK):
        return True

    print(respToStr[resp])
    return False

def upload(dev, path):
    device = central.Central(adapter_addr=dev.adapter, device_addr=dev.address)
    rx_char = device.add_characteristic(OTA_SERVICE_UUID, OTA_CHARACTERISTIC_UUID_RX)
    tx_char = device.add_characteristic(OTA_SERVICE_UUID, OTA_CHARACTERISTIC_UUID_TX)

    print("Connecting to " + dev.alias)
    device.connect()
    if not device.connected:
        print("Didn't connect to device!")
        return

    time = datetime.datetime.now()
    crc = 0
    uploaded_len = 0
    firmware_len = file_size(path)
    rx_char.value = int_to_u8_bytes(BEGIN) + int_to_u32_bytes(firmware_len)
    if (not handleResponse(u8_bytes_to_int(tx_char.value))):
        device.disconnect()
        return

    with open(path, 'rb') as f:
        while True:
            data = f.read(BUFFER_SIZE)
            if not data:
                break

            rx_char.value = int_to_u8_bytes(PACKAGE) + list(data)
            if (not handleResponse(u8_bytes_to_int(tx_char.value))):
                device.disconnect()
                return

            uploaded_len += len(data)
            print("Uploaded: " + str(uploaded_len) + "/" + str(firmware_len))
            crc = zlib.crc32(data, crc)

    rx_char.value = int_to_u8_bytes(END) + int_to_u32_bytes(crc)
    if (not handleResponse(u8_bytes_to_int(tx_char.value))):
        device.disconnect()
        return

    upload_time = datetime.datetime.now() - time
    print("Installing. Upload time: " + str(upload_time))
    rx_char.value = int_to_u8_bytes(INSTALL)
    print("Success!")

    device.disconnect()


if __name__ == '__main__':
    path = sys.argv[1]
    devices = scan_ota_devices()
    for device in devices:
        print("OTA Device Found!")
        upload(device, path)
        break