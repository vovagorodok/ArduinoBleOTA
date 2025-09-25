import os
import platform


def get_file_size(path):
    if os.path.isfile(path):
        file_info = os.stat(path)
        return file_info.st_size


def is_linux():
    return 'Linux' in platform.system()


def is_fedora():
    release = platform.freedesktop_os_release()
    return 'Fedora' in release['NAME']
