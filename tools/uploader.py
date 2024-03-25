#!/usr/bin/env python3
from bleak import BleakClient, BleakScanner
from time import sleep
import asyncio
import sys
import zlib
import os
import datetime
import platform


BLE_OTA_SERVICE_UUID = "15c155ca-36c5-11ed-adc0-9741d6a72f04"
BLE_OTA_CHARACTERISTIC_UUID_RX = "15c1564c-36c5-11ed-adc1-a3d6cf5cc2a4"
BLE_OTA_CHARACTERISTIC_UUID_TX = "15c156e2-36c5-11ed-adc2-7396d4fd413a"
BLE_OTA_CHARACTERISTIC_UUID_HW_NAME = "15c1576e-36c5-11ed-adc3-8799895de51e"
BLE_OTA_CHARACTERISTIC_UUID_HW_VER = "15c157fa-36c5-11ed-adc4-579c60267b47"
BLE_OTA_CHARACTERISTIC_UUID_SW_NAME = "15c15886-36c5-11ed-adc5-1bc0d0a6069d"
BLE_OTA_CHARACTERISTIC_UUID_SW_VER = "15c1591c-36c5-11ed-adc6-dbe9603dbf19"

OK = 0x00
NOK = 0x01
INCORRECT_FORMAT = 0x02
INCORRECT_FIRMWARE_SIZE = 0x03
CHECKSUM_ERROR = 0x04
INTERNAL_STORAGE_ERROR = 0x05
UPLOAD_DISABLED = 0x06

BEGIN = 0x10
PACKAGE = 0x11
END = 0x12

respToStr = {
    NOK: "Not ok",
    INCORRECT_FORMAT: "Incorrect format",
    INCORRECT_FIRMWARE_SIZE: "Incorrect firmware size",
    CHECKSUM_ERROR: "Checksum error",
    INTERNAL_STORAGE_ERROR: "Internal storage error",
    UPLOAD_DISABLED: "Upload disabled"
}

MTU_WRITE_OVERHEAD_BYTES_NUM = 3

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
    return int.to_bytes(value, U8_BYTES_NUM, 'little', signed=False)


def int_to_u32_bytes(value):
    return int.to_bytes(value, U32_BYTES_NUM, 'little', signed=False)


async def scan_ota_devices(timeout=5.0):
    devices_dict = await BleakScanner.discover(timeout=timeout, return_adv=True)

    for dev, adv in devices_dict.values():
        if BLE_OTA_SERVICE_UUID.lower() in adv.service_uuids:
            yield dev


def is_linux():
    return 'Linux' in platform.system()


def is_fedora():
    release = platform.freedesktop_os_release()
    return 'Fedora' in release['NAME']


async def acquire_mtu(client: BleakClient):
    if not is_linux():
        return

    # issue: https://github.com/hbldh/bleak/issues/1471
    if is_fedora():
        return

    from bleak.backends.bluezdbus.client import BleakClientBlueZDBus
    if type(client._backend) is not BleakClientBlueZDBus:
        return
    
    # in Linux acquire mtu should be called in order to have more than 23
    await client._backend._acquire_mtu()


async def get_mtu(client: BleakClient):
    is_fedora_linux = is_linux() and is_fedora()
    if not is_fedora_linux:
        return client.mtu_size


async def handle_response(resp):
    resp = bytes_to_int(resp)
    if resp == OK:
        return True

    print(respToStr[resp])
    return False


async def handle_begin_response(resp):
    head = resp[HEAD_POS]
    if head != OK:
        print(respToStr[head])
        return

    if len(resp) != BEGIN_RESP_BYTES_NUM:
        print("Incorrect begin responce")
        return
    return bytes_to_int(resp[ATTR_SIZE_POS:BUFFER_SIZE_POS]), bytes_to_int(resp[BUFFER_SIZE_POS:])


async def connect(dev):
    client = BleakClient(dev)

    print(f"Connecting to {dev.name}")
    if not await client.connect():
        print("Didn't connect to device!")
        return

    await acquire_mtu(client)

    service = client.services.get_service(BLE_OTA_SERVICE_UUID)
    rx_char = service.get_characteristic(BLE_OTA_CHARACTERISTIC_UUID_RX)
    tx_char = service.get_characteristic(BLE_OTA_CHARACTERISTIC_UUID_TX)
    hw_name_char = service.get_characteristic(BLE_OTA_CHARACTERISTIC_UUID_HW_NAME)
    hw_ver_char = service.get_characteristic(BLE_OTA_CHARACTERISTIC_UUID_HW_VER)
    sw_name_char = service.get_characteristic(BLE_OTA_CHARACTERISTIC_UUID_SW_NAME)
    sw_ver_char = service.get_characteristic(BLE_OTA_CHARACTERISTIC_UUID_SW_VER)

    print(", ".join([f"HW: {str(await client.read_gatt_char(hw_name_char), 'utf-8')}",
                     f"VER: {list(await client.read_gatt_char(hw_ver_char))}",
                     f"SW: {str(await client.read_gatt_char(sw_name_char), 'utf-8')}",
                     f"VER: {list(await client.read_gatt_char(sw_ver_char))}"]))

    return client, rx_char, tx_char


async def upload(client: BleakClient, rx_char, tx_char, path):
    crc = 0
    uploaded_len = 0
    firmware_len = file_size(path)
    current_buffer_len = 0

    if not firmware_len:
        print(f"File not exist: {path}")
        return False

    # issue: https://github.com/hbldh/bleak/issues/1501
    queue = asyncio.Queue(1)
    async def callback(char, array):
        await queue.put(array)
    await client.start_notify(tx_char, callback)

    begin_req = int_to_u8_bytes(BEGIN) + int_to_u32_bytes(firmware_len)
    await client.write_gatt_char(rx_char, begin_req)
    begin_resp = await handle_begin_response(await queue.get())
    if not begin_resp:
        return False
    attr_size, buffer_size = begin_resp
    
    mtu = await get_mtu(client)
    if (mtu):
        attr_size = min(attr_size, mtu - MTU_WRITE_OVERHEAD_BYTES_NUM)

    print(f"Begin upload: attr size: {attr_size}, buffer size: {buffer_size}")

    with open(path, 'rb') as f:
        while True:
            data = f.read(attr_size - HEAD_BYTES_NUM)
            if not len(data):
                break

            package = int_to_u8_bytes(PACKAGE) + data
            await client.write_gatt_char(rx_char, package)
            if current_buffer_len + len(data) > buffer_size:
                if not await handle_response(await queue.get()):
                    return False
                current_buffer_len = 0
            current_buffer_len += len(data)

            uploaded_len += len(data)
            crc = zlib.crc32(data, crc)
            print(f"Uploaded: {uploaded_len}/{firmware_len}")

    end_req = int_to_u8_bytes(END) + int_to_u32_bytes(crc)
    await client.write_gatt_char(rx_char, end_req)
    if not await handle_response(await queue.get()):
        return False

    return True


async def try_upload(client, rx_char, tx_char, path):
    time = datetime.datetime.now()

    if not await upload(client, rx_char, tx_char, path):
        return False

    upload_time = datetime.datetime.now() - time
    print(f"Installing. Upload time: {upload_time}")
    return True


async def connect_and_upload(dev, path):
    res = await connect(dev)
    if not res:
        return
    client, rx_char, tx_char = res

    if not await try_upload(client, rx_char, tx_char, path):
        await client.disconnect()
        return
    await client.disconnect()
    sleep(1)

    res = await connect(dev)
    if not res:
        return
    client, rx_char, tx_char = res

    await client.disconnect()
    print("Success!")


async def scan_and_upload(path):
    devices = list()

    print("Devices:")
    async for device in scan_ota_devices():
        print(f"{len(devices)}. [{device.address}] {device.name}")
        devices.append(device)

    if not len(devices):
        print("Device not found.")
        exit()

    user_input = input("Chose device [0]: ")

    try:
        device_num = int(user_input)
        if device_num >= len(devices) or device_num < 0:
            print("Incorrect device number.")
            exit()
    except ValueError:
        device_num = 0
        if len(user_input):
            print("Incorrect input.")
            exit()

    await connect_and_upload(devices[device_num], path)


def try_scan_and_upload(path):
    try:
        asyncio.run(scan_and_upload(path))
    except KeyboardInterrupt:
        print("User interrupt.")
    except Exception as e:
        print(e)


if __name__ == "__main__":
    try_scan_and_upload(sys.argv[1])
