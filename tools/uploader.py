#!/usr/bin/env python3
from bleak import BleakClient, BleakScanner
from time import sleep
from ble_ota import uuids
from ble_ota import consts
from ble_ota.messages import InitReq, InitResp
from ble_ota.messages import BeginReq, BeginResp
from ble_ota.messages import PackageInd, PackageReq, PackageResp
from ble_ota.messages import EndReq, EndResp
from ble_ota.messages import SignatureReq, SignatureResp
from ble_ota.messages import ErrorCode, parse_message_of_type, ERROR_TO_STR
from ble_ota.utils import get_file_size, is_linux, is_fedora, create_compressed_file, create_signature_file
from ble_ota.paths import Paths
import asyncio
import sys
import zlib
import datetime
import os


async def scan_ota_devices(timeout=5.0):
    devices_dict = await BleakScanner.discover(timeout=timeout, return_adv=True)

    for dev, adv in devices_dict.values():
        if uuids.BLE_OTA_SERVICE_UUID.lower() in adv.service_uuids:
            yield dev


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


async def connect(dev):
    client = BleakClient(dev)

    print(f"Connecting to {dev.name}")
    await client.connect()
    if not client.is_connected:
        print("Connection fail")
        return

    await acquire_mtu(client)

    service = client.services.get_service(uuids.BLE_OTA_SERVICE_UUID)
    tx_char = service.get_characteristic(uuids.BLE_OTA_CHARACTERISTIC_UUID_TX)
    rx_char = service.get_characteristic(uuids.BLE_OTA_CHARACTERISTIC_UUID_RX)
    mf_name_char = service.get_characteristic(uuids.BLE_OTA_CHARACTERISTIC_UUID_MF_NAME)
    hw_name_char = service.get_characteristic(uuids.BLE_OTA_CHARACTERISTIC_UUID_HW_NAME)
    hw_ver_char = service.get_characteristic(uuids.BLE_OTA_CHARACTERISTIC_UUID_HW_VER)
    sw_name_char = service.get_characteristic(uuids.BLE_OTA_CHARACTERISTIC_UUID_SW_NAME)
    sw_ver_char = service.get_characteristic(uuids.BLE_OTA_CHARACTERISTIC_UUID_SW_VER)

    if mf_name_char and hw_name_char and hw_ver_char and sw_name_char and sw_ver_char:
        mf = str(await client.read_gatt_char(mf_name_char), 'utf-8')
        hw = str(await client.read_gatt_char(hw_name_char), 'utf-8')
        sw = str(await client.read_gatt_char(sw_name_char), 'utf-8')
        hw_ver = ".".join(map(str, await client.read_gatt_char(hw_ver_char)))
        sw_ver = ".".join(map(str, await client.read_gatt_char(sw_ver_char)))
        print(f"Device: name: (mf: {mf}, hw: {hw}, sw: {sw}), ver: (hw: {hw_ver}, sw: {sw_ver})")
    else:
        print(f"Device info not available")

    return client, tx_char, rx_char


async def upload(paths: Paths, client: BleakClient, tx_char, rx_char):
    crc = 0
    uploaded_size = 0
    current_buffer_size = 0
    firmware_path = paths.firmware
    firmware_size = get_file_size(firmware_path)

    if not firmware_size:
        print(f"File not exist: {firmware_path}")
        return False

    # issue: https://github.com/hbldh/bleak/issues/1501
    queue = asyncio.Queue(1)
    async def callback(char, array):
        await queue.put(array)
    await client.start_notify(rx_char, callback)

    init_req = InitReq()
    await client.write_gatt_char(tx_char, init_req.to_bytes())
    init_resp = parse_message_of_type(await queue.get(), InitResp)

    if not init_resp.flags.upload:
        print(ERROR_TO_STR[ErrorCode.UPLOAD_DISABLED])
        return False

    if init_resp.flags.compression:
        compressed_path = firmware_path + ".zlib"
        create_compressed_file(firmware_path, compressed_path)
        compressed_size = get_file_size(compressed_path)
        upload_size = compressed_size
        firmware_path = compressed_path
        print(f"Firmware compressed: {firmware_size} -> {compressed_size}")
    else:
        compressed_size = firmware_size
        upload_size = firmware_size

    if init_resp.flags.signature:
        signature_path = firmware_path + ".sig"
        private_key_path = paths.private_key
        if not private_key_path or not os.path.isfile(private_key_path):
            print(f"Private key required")
            return False
        create_signature_file(firmware_path, signature_path, private_key_path)
        signature_size = get_file_size(signature_path)
        print(f"Signature created: {signature_size}")
    else:
        signature_path = None
        signature_size = 0

    mtu = await get_mtu(client)
    package_size = (mtu - consts.MESSAGE_OVERHEAD) if mtu else consts.MAX_U32
    buffer_size = consts.MAX_U32

    begin_req_flags = BeginReq.Flags(init_resp.flags.compression, init_resp.flags.checksum)
    begin_req = BeginReq(firmware_size, package_size, buffer_size, compressed_size, begin_req_flags)
    await client.write_gatt_char(tx_char, begin_req.to_bytes())
    begin_resp = parse_message_of_type(await queue.get(), BeginResp)

    package_size = begin_resp.package_size
    buffer_size = begin_resp.buffer_size

    print(f"Begin upload sizes: firmware: {firmware_size}, package: {package_size}, buffer: {buffer_size}, compressed: {compressed_size}")

    with open(firmware_path, 'rb') as f:
        while True:
            data = f.read(package_size)
            if not len(data):
                break

            if current_buffer_size + len(data) > buffer_size:
                package_req = PackageReq(data)
                await client.write_gatt_char(tx_char, package_req.to_bytes())
                parse_message_of_type(await queue.get(), PackageResp)
                current_buffer_size = 0
            else:
                package_ind = PackageInd(data)
                await client.write_gatt_char(tx_char, package_ind.to_bytes())

            current_buffer_size += len(data)
            uploaded_size += len(data)
            if init_resp.flags.checksum:
                crc = zlib.crc32(data, crc)
            print(f"Uploaded: {uploaded_size}/{upload_size}")

    if signature_size:
        print(f"Signature upload")
        uploaded_size = 0
        with open(signature_path, 'rb') as f:
            while True:
                data = f.read(package_size)
                if not len(data):
                    break

                signature_req = SignatureReq(data)
                await client.write_gatt_char(tx_char, signature_req.to_bytes())
                parse_message_of_type(await queue.get(), SignatureResp)

                uploaded_size += len(data)
                print(f"Uploaded: {uploaded_size}/{signature_size}")

    end_req = EndReq(crc)
    await client.write_gatt_char(tx_char, end_req.to_bytes())
    parse_message_of_type(await queue.get(), EndResp)

    return True


async def try_upload(paths: Paths, client, tx_char, rx_char):
    time = datetime.datetime.now()

    if not await upload(paths, client, tx_char, rx_char):
        return False

    upload_time = datetime.datetime.now() - time
    print(f"Installing. Upload time: {upload_time}")
    return True


async def connect_and_upload(paths: Paths, dev):
    res = await connect(dev)
    if not res:
        return
    client, tx_char, rx_char = res

    if not await try_upload(paths, client, tx_char, rx_char):
        await client.disconnect()
        return
    await client.disconnect()
    sleep(1)

    res = await connect(dev)
    if not res:
        return
    client, tx_char, rx_char = res

    await client.disconnect()
    print("Success!")


async def scan_and_upload(paths: Paths):
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

    await connect_and_upload(paths, devices[device_num])


def try_scan_and_upload(paths: Paths):
    try:
        asyncio.run(scan_and_upload(paths))
    except KeyboardInterrupt:
        print("User interrupt.")
    except Exception as e:
        print(e)


if __name__ == "__main__":
    try_scan_and_upload(Paths.parse(sys.argv))
