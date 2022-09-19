from bluezero import adapter
from bluezero import central
from time import sleep
import sys
import zlib
import os
import datetime

OTA_SERVICE_UUID                = "15c155ca-36c5-11ed-adc0-9741d6a72f04"
OTA_CHARACTERISTIC_UUID_RX      = "15c1564c-36c5-11ed-adc1-a3d6cf5cc2a4"
OTA_CHARACTERISTIC_UUID_TX      = "15c156e2-36c5-11ed-adc2-7396d4fd413a"
OTA_CHARACTERISTIC_UUID_HW_NAME = "15c1576e-36c5-11ed-adc3-8799895de51e"
OTA_CHARACTERISTIC_UUID_HW_VER  = "15c157fa-36c5-11ed-adc4-579c60267b47"
OTA_CHARACTERISTIC_UUID_SW_NAME = "15c15886-36c5-11ed-adc5-1bc0d0a6069d"
OTA_CHARACTERISTIC_UUID_SW_VER  = "15c1591c-36c5-11ed-adc6-dbe9603dbf19"

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
    if resp == OK:
        return True

    print(respToStr[resp])
    return False

def connect(dev):
    device = central.Central(adapter_addr=dev.adapter, device_addr=dev.address)
    rx_char = device.add_characteristic(OTA_SERVICE_UUID, OTA_CHARACTERISTIC_UUID_RX)
    tx_char = device.add_characteristic(OTA_SERVICE_UUID, OTA_CHARACTERISTIC_UUID_TX)
    hw_name_char = device.add_characteristic(OTA_SERVICE_UUID, OTA_CHARACTERISTIC_UUID_HW_NAME)
    hw_ver_char = device.add_characteristic(OTA_SERVICE_UUID, OTA_CHARACTERISTIC_UUID_HW_VER)
    sw_name_char = device.add_characteristic(OTA_SERVICE_UUID, OTA_CHARACTERISTIC_UUID_SW_NAME)
    sw_ver_char = device.add_characteristic(OTA_SERVICE_UUID, OTA_CHARACTERISTIC_UUID_SW_VER)

    print("Connecting to " + dev.alias)
    device.connect()
    if not device.connected:
        print("Didn't connect to device!")
        return

    print(", ".join(["HW: "  + str(bytearray(hw_name_char.value), 'utf-8'),
                     "VER: " + str(list(bytearray(hw_ver_char.value))),
                     "SW: "  + str(bytearray(sw_name_char.value), 'utf-8'),
                     "VER: " + str(list(bytearray(sw_ver_char.value)))]))

    return device, rx_char, tx_char

def upload(rx_char, tx_char, path):
    time = datetime.datetime.now()
    crc = 0
    uploaded_len = 0
    firmware_len = file_size(path)

    rx_char.value = int_to_u8_bytes(BEGIN) + int_to_u32_bytes(firmware_len)
    if not handleResponse(u8_bytes_to_int(tx_char.value)):
        return False

    with open(path, 'rb') as f:
        while True:
            data = f.read(BUFFER_SIZE)
            if not data:
                break

            rx_char.value = int_to_u8_bytes(PACKAGE) + list(data)
            if not handleResponse(u8_bytes_to_int(tx_char.value)):
                return False

            uploaded_len += len(data)
            print("Uploaded: " + str(uploaded_len) + "/" + str(firmware_len))
            crc = zlib.crc32(data, crc)

    rx_char.value = int_to_u8_bytes(END) + int_to_u32_bytes(crc)
    if not handleResponse(u8_bytes_to_int(tx_char.value)):
        return False

    upload_time = datetime.datetime.now() - time
    print("Installing. Upload time: " + str(upload_time))
    return True

def connect_and_upload(dev, path):
    res = connect(dev)
    if not res:
        return
    device, rx_char, tx_char = res

    if not upload(rx_char, tx_char, path):
        device.disconnect()
        return
    device.disconnect()
    sleep(1)

    res = connect(dev)
    if not res:
        return
    device, rx_char, tx_char = res

    device.disconnect()
    print("Success!")

if __name__ == '__main__':
    path = sys.argv[1]
    devices = scan_ota_devices()
    for device in devices:
        print("OTA Device Found!")
        connect_and_upload(device, path)
        break