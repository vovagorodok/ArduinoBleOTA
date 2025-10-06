#!/usr/bin/env python3
import json
import os

from uploader import try_scan_and_upload
from ble_ota.utils import create_private_key_path
from ble_ota.paths import InputPaths


def find_abs_dir_that_contains(abs_dir, relative_path):
    abs_dir, tail = os.path.split(abs_dir)
    while tail:
        abs_path = os.path.join(abs_dir, relative_path)
        if os.path.exists(abs_path) and os.path.isfile(abs_path):
            return abs_dir
        abs_dir, tail = os.path.split(abs_dir)


def load_json_ignoring_comments(path):
    def is_not_comment(line):
        return not line.startswith('//')

    json_file = open(path, 'r')
    without_comments = ''.join(filter(is_not_comment, json_file))
    return json.loads(without_comments)


def find_firmware_bin_path(script_dir):
    relative_vscode_launch_path = os.path.join('.vscode', 'launch.json')

    project_root_dir = find_abs_dir_that_contains(script_dir, relative_vscode_launch_path)
    if not project_root_dir:
        print("Use VS Code with PlatformIO plugin")
        exit()

    vscode_launch_path = os.path.join(project_root_dir, relative_vscode_launch_path)
    vscode_launch_json = load_json_ignoring_comments(vscode_launch_path)
    current_env = vscode_launch_json['configurations'][0]['projectEnvName']
    firmware_bin_path = os.path.join(project_root_dir, '.pio', 'build', current_env, 'firmware.bin')
    return firmware_bin_path


if __name__ == '__main__':
    script_dir = os.path.dirname(os.path.abspath(__file__))
    paths = InputPaths(find_firmware_bin_path(script_dir), create_private_key_path(script_dir))
    try_scan_and_upload(paths)
