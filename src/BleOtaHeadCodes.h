#pragma once

#define OK 0x00
#define NOK 0x01
#define INCORRECT_FORMAT 0x02
#define INCORRECT_FIRMWARE_SIZE 0x03
#define CHECKSUM_ERROR 0x04
#define INTERNAL_STORAGE_ERROR 0x05
#define UPLOAD_DISABLED 0x06

#define BEGIN 0x10
#define PACKAGE 0x11
#define END 0x12

#define SET_PIN 0x20
#define REMOVE_PIN 0x21
