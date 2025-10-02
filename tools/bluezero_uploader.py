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
from ble_ota.messages import SignatureReq, SignatureResp
from ble_ota.messages import ErrorCode, parse_message_of_type, ERROR_TO_STR
from ble_ota.utils import get_file_size, create_compressed_file, create_signature_file
from ble_ota.paths import Paths
import sys
import zlib
import datetime
import os


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
        print("Connection fail")
        return

    try:
        mf = str(bytearray(mf_name_char.value), 'utf-8')
        hw = str(bytearray(hw_name_char.value), 'utf-8')
        sw = str(bytearray(sw_name_char.value), 'utf-8')
        hw_ver = '.'.join(map(str, bytearray(hw_ver_char.value)))
        sw_ver = '.'.join(map(str, bytearray(sw_ver_char.value)))
        print(f"Device: name: (mf: {mf}, hw: {hw}, sw: {sw}), ver: (hw: {hw_ver}, sw: {sw_ver})")
    except Exception:
        print("Device info not available")

    return device, tx_char, rx_char


def send(tx_char, data: bytes):
    tx_char.value = list(data)


def receive(rx_char):
    return bytes(rx_char.value)


def upload(paths: Paths, tx_char, rx_char):
    crc = 0
    uploaded_size = 0
    current_buffer_size = 0
    firmware_path = paths.firmware
    firmware_size = get_file_size(firmware_path)

    if not firmware_size:
        print(f"File not exist: {firmware_path}")
        return False

    init_req = InitReq()
    send(tx_char, init_req.to_bytes())
    init_resp = parse_message_of_type(receive(rx_char), InitResp)

    if not init_resp.flags.upload:
        print(ERROR_TO_STR[ErrorCode.UPLOAD_DISABLED])
        return False

    if init_resp.flags.signature:
        signature_path = firmware_path + '.sig'
        private_key_path = paths.private_key
        if not private_key_path or not os.path.isfile(private_key_path):
            print("Private key required")
            return False
        create_signature_file(firmware_path, signature_path, private_key_path)
        signature_size = get_file_size(signature_path)
        print(f"Signature created: {signature_size}")
    else:
        signature_path = None
        signature_size = 0

    if init_resp.flags.compression:
        compressed_path = firmware_path + '.zlib'
        create_compressed_file(firmware_path, compressed_path)
        compressed_size = get_file_size(compressed_path)
        upload_size = compressed_size
        firmware_path = compressed_path
        print(f"Firmware compressed: {firmware_size} -> {compressed_size}")
    else:
        compressed_size = firmware_size
        upload_size = firmware_size

    package_size = consts.MAX_U32
    buffer_size = consts.MAX_U32

    begin_req_flags = BeginReq.Flags(init_resp.flags.compression, init_resp.flags.checksum)
    begin_req = BeginReq(firmware_size, package_size, buffer_size, compressed_size, begin_req_flags)
    send(tx_char, begin_req.to_bytes())
    begin_resp = parse_message_of_type(receive(rx_char), BeginResp)

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
                send(tx_char, package_req.to_bytes())
                parse_message_of_type(receive(rx_char), PackageResp)
                current_buffer_size = 0
            else:
                package_ind = PackageInd(data)
                send(tx_char, package_ind.to_bytes())
    
            current_buffer_size += len(data)
            uploaded_size += len(data)
            if init_resp.flags.checksum:
                crc = zlib.crc32(data, crc)
            print(f"Uploaded: {uploaded_size}/{upload_size}")

    if signature_size:
        print("Signature upload")
        uploaded_size = 0
        with open(signature_path, 'rb') as f:
            while True:
                data = f.read(package_size)
                if not len(data):
                    break

                signature_req = SignatureReq(data)
                send(tx_char, signature_req.to_bytes())
                parse_message_of_type(receive(rx_char), SignatureResp)

                uploaded_size += len(data)
                print(f"Uploaded: {uploaded_size}/{signature_size}")

    end_req = EndReq(crc)
    send(tx_char, end_req.to_bytes())
    parse_message_of_type(receive(rx_char), EndResp)

    return True


def try_upload(paths: Paths, tx_char, rx_char):
    time = datetime.datetime.now()

    try:
        if not upload(paths, tx_char, rx_char):
            return False
    except Exception as e:
        print(e)
        return False

    upload_time = datetime.datetime.now() - time
    print(f"Installing. Upload time: {upload_time}")
    return True


def connect_and_upload(paths: Paths, dev):
    res = connect(dev)
    if not res:
        return
    device, tx_char, rx_char = res

    if not try_upload(paths, tx_char, rx_char):
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


def scan_and_upload(paths: Paths):
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

    connect_and_upload(paths, devices[device_num])


def try_scan_and_upload(paths: Paths):
    try:
        scan_and_upload(paths)
    except KeyboardInterrupt:
        print("User interrupt.")
    except Exception as e:
        print(e)


if __name__ == '__main__':
    try_scan_and_upload(Paths.parse(sys.argv))
