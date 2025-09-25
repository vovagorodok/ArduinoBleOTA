#!/usr/bin/env python3
from bluezero import adapter
from bluezero import central
from time import sleep
from ble_ota import uuids
from ble_ota import consts
from ble_ota.messages import InitReq, InitResp
from ble_ota.messages import BeginReq, BeginResp
from ble_ota.messages import PackageInd, PackageReq, PackageResp
from ble_ota.messages import EndReq, EndResp
from ble_ota.messages import ErrorCode, parse_message_of_type, errToStr
from ble_ota.utils import get_file_size
import sys
import zlib
import datetime


def scan_ota_devices(adapter_address=None, timeout=5.0):
    for dongle in adapter.Adapter.available():
        if adapter_address and adapter_address.upper() != dongle.address():
            continue

        dongle.nearby_discovery(timeout=timeout)

        for dev in central.Central.available(dongle.address):
            if uuids.BLE_OTA_SERVICE_UUID.lower() in dev.uuids:
                yield dev


def connect(dev):
    device = central.Central(adapter_addr=dev.adapter, device_addr=dev.address)
    tx_char = device.add_characteristic(
        uuids.BLE_OTA_SERVICE_UUID, uuids.BLE_OTA_CHARACTERISTIC_UUID_TX)
    rx_char = device.add_characteristic(
        uuids.BLE_OTA_SERVICE_UUID, uuids.BLE_OTA_CHARACTERISTIC_UUID_RX)
    mf_name_char = device.add_characteristic(
        uuids.BLE_OTA_SERVICE_UUID, uuids.BLE_OTA_CHARACTERISTIC_UUID_MF_NAME)
    hw_name_char = device.add_characteristic(
        uuids.BLE_OTA_SERVICE_UUID, uuids.BLE_OTA_CHARACTERISTIC_UUID_HW_NAME)
    hw_ver_char = device.add_characteristic(
        uuids.BLE_OTA_SERVICE_UUID, uuids.BLE_OTA_CHARACTERISTIC_UUID_HW_VER)
    sw_name_char = device.add_characteristic(
        uuids.BLE_OTA_SERVICE_UUID, uuids.BLE_OTA_CHARACTERISTIC_UUID_SW_NAME)
    sw_ver_char = device.add_characteristic(
        uuids.BLE_OTA_SERVICE_UUID, uuids.BLE_OTA_CHARACTERISTIC_UUID_SW_VER)

    print(f"Connecting to {dev.alias}")
    device.connect()
    if not device.connected:
        print("Didn't connect to device!")
        return

    try:
        mf = str(bytearray(mf_name_char.value), 'utf-8')
        hw = str(bytearray(hw_name_char.value), 'utf-8')
        sw = str(bytearray(sw_name_char.value), 'utf-8')
        hw_ver = ".".join(map(str, bytearray(hw_ver_char.value)))
        sw_ver = ".".join(map(str, bytearray(sw_ver_char.value)))
        print(f"Device: name: (mf: {mf}, hw: {hw}, sw: {sw}), ver: (hw: {hw_ver}, sw: {sw_ver})")
    except Exception as e:
        print(e)
        return

    return device, tx_char, rx_char


def send(tx_char, data: bytes):
    tx_char.value = list(data)


def receive(rx_char):
    return bytes(rx_char.value)


def upload(tx_char, rx_char, path):
    crc = 0
    uploaded_size = 0
    firmware_size = get_file_size(path)
    current_buffer_size = 0

    if not firmware_size:
        print(f"File not exist: {path}")
        return False

    init_req = InitReq()
    send(tx_char, init_req.to_bytes())
    init_resp = parse_message_of_type(receive(rx_char), InitResp)

    if not init_resp.flags.upload:
        print(errToStr[ErrorCode.UPLOAD_DISABLED])
        return False

    if init_resp.flags.compression:
        compressed_path = path + ".zlib"
        with open(path, "rb") as fin, open(compressed_path, "wb") as fout:
            fout.write(zlib.compress(fin.read()))
        compressed_size = get_file_size(compressed_path)
        file_size = compressed_size
        path = compressed_path
        print(f"Firmware compressed: {firmware_size} -> {compressed_size}")
    else:
        compressed_size = firmware_size
        file_size = firmware_size

    package_size = consts.MAX_U32
    buffer_size = consts.MAX_U32

    begin_req_flags = BeginReq.Flags(init_resp.flags.compression, init_resp.flags.checksum)
    begin_req = BeginReq(firmware_size, package_size, buffer_size, compressed_size, begin_req_flags)
    send(tx_char, begin_req.to_bytes())
    begin_resp = parse_message_of_type(receive(rx_char), BeginResp)

    package_size = begin_resp.package_size
    buffer_size = begin_resp.buffer_size

    print(f"Begin upload sizes: firmware: {firmware_size}, package: {package_size}, buffer: {buffer_size}, compressed: {compressed_size}")

    with open(path, 'rb') as f:
        while True:
            data = f.read(package_size)
            if not len(data):
                break

            if current_buffer_size + len(data) > buffer_size:
                package_req = PackageReq(data)
                send(tx_char, package_req.to_bytes())
                parse_message_of_type(receive(rx_char), PackageResp)
                current_buffer_size = 0
            else:
                package_ind = PackageInd(data)
                send(tx_char, package_ind.to_bytes())
    
            current_buffer_size += len(data)
            uploaded_size += len(data)
            crc = zlib.crc32(data, crc)
            print(f"Uploaded: {uploaded_size}/{file_size}")

    end_req = EndReq(crc)
    send(tx_char, end_req.to_bytes())
    parse_message_of_type(receive(rx_char), EndResp)

    return True


def try_upload(tx_char, rx_char, path):
    time = datetime.datetime.now()

    try:
        if not upload(tx_char, rx_char, path):
            return False
    except Exception as e:
        print(e)
        return False

    upload_time = datetime.datetime.now() - time
    print(f"Installing. Upload time: {upload_time}")
    return True


def connect_and_upload(dev, path):
    res = connect(dev)
    if not res:
        return
    device, tx_char, rx_char = res

    if not try_upload(tx_char, rx_char, path):
        device.disconnect()
        return
    device.disconnect()
    sleep(1)

    res = connect(dev)
    if not res:
        return
    device, tx_char, rx_char = res

    device.disconnect()
    print("Success!")


def scan_and_upload(path):
    devices = list()

    print("Devices:")
    for device in scan_ota_devices():
        print(f"{len(devices)}. [{device.address}] {device.alias}")
        devices.append(device)

    if not len(devices):
        print("Device not found")
        exit()

    user_input = input("Chose device [0]: ")

    try:
        device_num = int(user_input)
        if device_num >= len(devices) or device_num < 0:
            print("Incorrect device number")
            exit()
    except ValueError:
        device_num = 0
        if len(user_input):
            print("Incorrect input")
            exit()

    connect_and_upload(devices[device_num], path)


if __name__ == '__main__':
    scan_and_upload(sys.argv[1])
