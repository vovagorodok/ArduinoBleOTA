import os
import platform
import subprocess
import zlib


def get_file_size(path):
    if os.path.isfile(path):
        file_info = os.stat(path)
        return file_info.st_size


def is_linux():
    return 'Linux' in platform.system()


def is_fedora():
    release = platform.freedesktop_os_release()
    return 'Fedora' in release['NAME']


def create_private_key_path(script_dir: str):
    return os.path.join(script_dir, 'signature_keys', 'priv_key.pem')


def create_compressed_file(firmware_path: str, compressed_path: str):
    with open(firmware_path, "rb") as fin, open(compressed_path, "wb") as fout:
        fout.write(zlib.compress(fin.read()))


def create_signature_file(firmware_path: str, signature_path: str, private_key_path: str):
    cmd = [
        "openssl", "dgst",
        "-sign", private_key_path,
        "-keyform", "PEM",
        "-sha256",
        "-out", signature_path,
        "-binary", firmware_path
    ]
    subprocess.run(cmd, check=True)