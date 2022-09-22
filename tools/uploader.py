from bluezero import adapter
from bluezero import central
from time import sleep
import sys
import zlib
import os
import datetime

BLE_OTA_SERVICE_UUID                = "15c155ca-36c5-11ed-adc0-9741d6a72f04"
BLE_OTA_CHARACTERISTIC_UUID_RX      = "15c1564c-36c5-11ed-adc1-a3d6cf5cc2a4"
BLE_OTA_CHARACTERISTIC_UUID_TX      = "15c156e2-36c5-11ed-adc2-7396d4fd413a"
BLE_OTA_CHARACTERISTIC_UUID_HW_NAME = "15c1576e-36c5-11ed-adc3-8799895de51e"
BLE_OTA_CHARACTERISTIC_UUID_HW_VER  = "15c157fa-36c5-11ed-adc4-579c60267b47"
BLE_OTA_CHARACTERISTIC_UUID_SW_NAME = "15c15886-36c5-11ed-adc5-1bc0d0a6069d"
BLE_OTA_CHARACTERISTIC_UUID_SW_VER  = "15c1591c-36c5-11ed-adc6-dbe9603dbf19"

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

U8_BYTES_NUM = 1
U32_BYTES_NUM = 4
HEAD_BYTES_NUM = U8_BYTES_NUM
ATTR_SIZE_BYTES_NUM = U32_BYTES_NUM
BUFFER_SIZE_BYTES_NUM = U32_BYTES_NUM
BEGIN_RESP_BYTES_NUM = HEAD_BYTES_NUM + ATTR_SIZE_BYTES_NUM + BUFFER_SIZE_BYTES_NUM
HEAD_POS = 0
ATTR_SIZE_POS = HEAD_POS + HEAD_BYTES_NUM
BUFFER_SIZE_POS = ATTR_SIZE_POS + ATTR_SIZE_BYTES_NUM

def file_size(path):
    if os.path.isfile(path):
        file_info = os.stat(path)
        return file_info.st_size

def bytes_to_int(value):
    return int.from_bytes(value, 'little', signed=False)

def int_to_u8_bytes(value):
    return list(int.to_bytes(value, U8_BYTES_NUM, 'little', signed=False))

def int_to_u32_bytes(value):
    return list(int.to_bytes(value, U32_BYTES_NUM, 'little', signed=False))

def scan_ota_devices(adapter_address=None, timeout=5.0):
    for dongle in adapter.Adapter.available():
        if adapter_address and adapter_address.upper() != dongle.address():
            continue

        dongle.nearby_discovery(timeout=timeout)

        for dev in central.Central.available(dongle.address):
            if BLE_OTA_SERVICE_UUID.lower() in dev.uuids:
                yield dev

def handleResponse(resp):
    resp = bytes_to_int(resp)
    if resp == OK:
        return True

    print(respToStr[resp])
    return False

def handleBeginResponse(resp):
    respList = list(bytearray(resp))
    head = respList[HEAD_POS]
    if head != OK:
        print(respToStr[resp])
        return

    if len(respList) != BEGIN_RESP_BYTES_NUM:
        print("Incorrect begin responce")
        return
    return bytes_to_int(respList[ATTR_SIZE_POS:BUFFER_SIZE_POS]), bytes_to_int(respList[BUFFER_SIZE_POS:])

def connect(dev):
    device = central.Central(adapter_addr=dev.adapter, device_addr=dev.address)
    rx_char = device.add_characteristic(BLE_OTA_SERVICE_UUID, BLE_OTA_CHARACTERISTIC_UUID_RX)
    tx_char = device.add_characteristic(BLE_OTA_SERVICE_UUID, BLE_OTA_CHARACTERISTIC_UUID_TX)
    hw_name_char = device.add_characteristic(BLE_OTA_SERVICE_UUID, BLE_OTA_CHARACTERISTIC_UUID_HW_NAME)
    hw_ver_char = device.add_characteristic(BLE_OTA_SERVICE_UUID, BLE_OTA_CHARACTERISTIC_UUID_HW_VER)
    sw_name_char = device.add_characteristic(BLE_OTA_SERVICE_UUID, BLE_OTA_CHARACTERISTIC_UUID_SW_NAME)
    sw_ver_char = device.add_characteristic(BLE_OTA_SERVICE_UUID, BLE_OTA_CHARACTERISTIC_UUID_SW_VER)

    print("Connecting to " + dev.alias)
    device.connect()
    if not device.connected:
        print("Didn't connect to device!")
        return

    try:
        print(", ".join(["HW: "  + str(bytearray(hw_name_char.value), 'utf-8'),
                        "VER: " + str(list(bytearray(hw_ver_char.value))),
                        "SW: "  + str(bytearray(sw_name_char.value), 'utf-8'),
                        "VER: " + str(list(bytearray(sw_ver_char.value)))]))
    except:
        print("No response from device")
        return


    return device, rx_char, tx_char

def upload(rx_char, tx_char, path):
    crc = 0
    uploaded_len = 0
    firmware_len = file_size(path)
    current_buffer_len = 0

    rx_char.value = int_to_u8_bytes(BEGIN) + int_to_u32_bytes(firmware_len)
    begin_resp = handleBeginResponse(tx_char.value)
    if not begin_resp:
        return False
    attr_size, buffer_size = begin_resp
    print("Begin upload: attr size: " + str(attr_size) + ", buffer size: " + str(buffer_size))

    with open(path, 'rb') as f:
        while True:
            data = f.read(attr_size - HEAD_BYTES_NUM)
            if not data:
                break

            rx_char.value = int_to_u8_bytes(PACKAGE) + list(data)
            if current_buffer_len + len(data) > buffer_size:
                if not handleResponse(tx_char.value):
                    return False
                current_buffer_len = 0
            current_buffer_len += len(data)

            uploaded_len += len(data)
            crc = zlib.crc32(data, crc)
            print("Uploaded: " + str(uploaded_len) + "/" + str(firmware_len))

    rx_char.value = int_to_u8_bytes(END) + int_to_u32_bytes(crc)
    if not handleResponse(tx_char.value):
        return False

    return True

def try_upload(rx_char, tx_char, path):
    time = datetime.datetime.now()

    try:
        if not upload(rx_char, tx_char, path):
            return False
    except:
        print("No response from device")
        return False

    upload_time = datetime.datetime.now() - time
    print("Installing. Upload time: " + str(upload_time))
    return True

def connect_and_upload(dev, path):
    res = connect(dev)
    if not res:
        return
    device, rx_char, tx_char = res

    if not try_upload(rx_char, tx_char, path):
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