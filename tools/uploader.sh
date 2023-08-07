#!/bin/bash

SCRIPT_DIR=$( cd $( dirname $0 ) && pwd )
SCRIPT_PATH=$SCRIPT_DIR/uploader.py
VSCODE_LAUNCH_PATH=$SCRIPT_DIR/../.vscode/launch.json
CURRENT_ENV=$( awk '/projectEnvName/ {print $2; exit}' $VSCODE_LAUNCH_PATH | sed 's/[",]//g' )
FIRMWARE_BIN_PATH=$SCRIPT_DIR/../.pio/build/$CURRENT_ENV/firmware.bin

python3 $SCRIPT_PATH $FIRMWARE_BIN_PATH