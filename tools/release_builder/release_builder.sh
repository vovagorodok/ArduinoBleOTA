#!/bin/bash

SCRIPT_DIR=$( cd $( dirname $0 ) && pwd )
OUT_DIR=$SCRIPT_DIR/out
PROJECT_DIR=$SCRIPT_DIR/../..
PIO_BUILD_DIR=$PROJECT_DIR/.pio/build
EXAMPLE_PATH=$PROJECT_DIR/examples/full/main.ino

source ~/.platformio/penv/bin/activate

rm -rf $OUT_DIR
mkdir -p $OUT_DIR
cd $PROJECT_DIR

change_example() {
    FROM=$1
    TO=$2
    sed -i "s/$FROM/$TO/g" $EXAMPLE_PATH
}

build_firmware() {
    BUILD_PLATFORM=$1
    BUILD_ENV=$2
    NEW_SW_NAME_AND_VER=$3
    NEW_SW_NAME=$4
    NEW_SW_VER=$5
    NEW_HW_VER=$6
    OLD_SW_NAME="Example SW"
    OLD_SW_VER="SW_VER {1, 0, 0}"
    OLD_HW_VER="HW_VER {1, 0, 0}"
    BIN_PATH=$PIO_BUILD_DIR/$BUILD_ENV/firmware.bin

    change_example "$OLD_SW_VER" "$NEW_SW_VER"
    change_example "$OLD_HW_VER" "$NEW_HW_VER"
    change_example "$OLD_SW_NAME" "$NEW_SW_NAME"

    pio run -e $BUILD_ENV
    mv $BIN_PATH $OUT_DIR/${BUILD_PLATFORM}_${NEW_SW_NAME_AND_VER}_firmware.bin

    change_example "$NEW_SW_VER" "$OLD_SW_VER"
    change_example "$NEW_HW_VER" "$OLD_HW_VER"
    change_example "$NEW_SW_NAME" "$OLD_SW_NAME"
}

build_firmware_for_platform() {
    BUILD_PLATFORM=$1
    BUILD_ENV=$2

    build_firmware $BUILD_PLATFORM $BUILD_ENV "example_v1.0.0" "Example SW" "SW_VER {1, 0, 0}" "HW_VER {1, 0, 0}"
    build_firmware $BUILD_PLATFORM $BUILD_ENV "example_v1.1.0" "Example SW" "SW_VER {1, 1, 0}" "HW_VER {1, 0, 0}"
    build_firmware $BUILD_PLATFORM $BUILD_ENV "example_v1.1.2" "Example SW" "SW_VER {1, 1, 2}" "HW_VER {1, 0, 0}"
    build_firmware $BUILD_PLATFORM $BUILD_ENV "example_v2.0.0" "Example SW" "SW_VER {2, 0, 0}" "HW_VER {2, 0, 0}"
    build_firmware $BUILD_PLATFORM $BUILD_ENV "experimental_v1.1.2" "Experimental SW" "SW_VER {1, 1, 2}" "HW_VER {1, 0, 0}"
    build_firmware $BUILD_PLATFORM $BUILD_ENV "other_v1.6.6" "Other SW" "SW_VER {1, 6, 6}" "HW_VER {1, 0, 0}"
}

build_firmware_for_platform "esp32" "esp32dev"
build_firmware_for_platform "atmelsam" "nano_33_iot"