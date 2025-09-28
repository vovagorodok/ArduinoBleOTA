from dataclasses import dataclass
from enum import IntEnum
from typing import Type, TypeVar
from ble_ota import consts
import struct


class HeaderCode(IntEnum):
    INIT_REQ = 0x01
    INIT_RESP = 0x02
    BEGIN_REQ = 0x03
    BEGIN_RESP = 0x04
    PACKAGE_IND = 0x05
    PACKAGE_REQ = 0x06
    PACKAGE_RESP = 0x07
    END_REQ = 0x08
    END_RESP = 0x09
    ERROR_IND = 0x10
    UPLOAD_ENABLE_IND = 0x11
    UPLOAD_DISABLE_IND = 0x12
    SET_PIN_REQ = 0x20
    SET_PIN_RESP = 0x21
    REMOVE_PIN_REQ = 0x22
    REMOVE_PIN_RESP = 0x23
    SIGNATURE_REQ = 0x30
    SIGNATURE_RESP = 0x31


class ErrorCode(IntEnum):
    OK = 0x00
    NOK = 0x01
    INCORRECT_FORMAT = 0x02
    INCORRECT_FIRMWARE_SIZE = 0x03
    INTERNAL_STORAGE_ERROR = 0x04
    UPLOAD_DISABLED = 0x10
    UPLOAD_RUNNING = 0x11
    UPLOAD_STOPPED = 0x12
    INSTALL_RUNNING = 0x13
    BUFFER_DISABLED = 0x20
    BUFFER_OVERFLOW = 0x21
    COMPRESSION_NOT_SUPPORTED = 0x30
    INCORRECT_COMPRESSION = 0x31
    INCORRECT_COMPRESSED_SIZE = 0x32
    INCORRECT_COMPRESSION_CHECKSUM = 0x33
    INCORRECT_COMPRESSION_PARAM = 0x34
    INCORRECT_COMPRESSION_END = 0x35
    CHECKSUM_NOT_SUPPORTED = 0x40
    INCORRECT_CHECKSUM = 0x41
    SIGNATURE_NOT_SUPPORTED = 0x50
    INCORRECT_SIGNATURE = 0x51
    INCORRECT_SIGNATURE_SIZE = 0x52


@dataclass
class Message:
    header: HeaderCode

    def to_bytes(self) -> bytes:
        return struct.pack("<B", self.header)

    @classmethod
    def from_bytes(cls, data: bytes):
        return cls()

    @staticmethod
    def bools_to_byte(bits: tuple[bool, ...]) -> int:
        value = 0
        for i, bit in enumerate(bits):
            value |= (bit << i)
        return value & consts.MAX_U8

    @staticmethod
    def byte_to_bools(value: int, count: int) -> tuple[bool, ...]:
        return tuple(bool(value >> i & 1) for i in range(count))


@dataclass
class InitReq(Message):
    def __init__(self):
        super().__init__(HeaderCode.INIT_REQ)


@dataclass
class InitResp(Message):
    @dataclass
    class Flags:
        compression: bool
        checksum: bool
        upload: bool
        pin: bool
        signature: bool

        def to_byte(self) -> int:
            return Message.bools_to_byte((
                self.compression,
                self.checksum,
                self.upload,
                self.pin,
                self.signature))

        @classmethod
        def from_byte(cls, value: int):
            compression, checksum, upload, pin, signature = Message.byte_to_bools(value, 5)
            return cls(compression, checksum, upload, pin, signature)

    flags: Flags

    def __init__(self, flags: Flags):
        super().__init__(HeaderCode.INIT_RESP)
        self.flags = flags

    def to_bytes(self) -> bytes:
        return struct.pack("<BB", self.header, self.flags.to_byte())

    @classmethod
    def from_bytes(cls, data: bytes):
        _, flags_byte = struct.unpack("<BB", data)
        flags = InitResp.Flags.from_byte(flags_byte)
        return cls(flags)


@dataclass
class BeginReq(Message):
    @dataclass
    class Flags:
        compression: bool
        checksum: bool

        def to_byte(self) -> int:
            return Message.bools_to_byte((self.compression, self.checksum))

        @classmethod
        def from_byte(cls, value: int):
            compression, checksum = Message.byte_to_bools(value, 2)
            return cls(compression, checksum)

    firmware_size: int
    package_size: int
    buffer_size: int
    compressed_size: int
    flags: Flags

    def __init__(self, firmware_size, package_size, buffer_size, compressed_size, flags: Flags):
        super().__init__(HeaderCode.BEGIN_REQ)
        self.firmware_size = firmware_size
        self.package_size = package_size
        self.buffer_size = buffer_size
        self.compressed_size = compressed_size
        self.flags = flags

    def to_bytes(self) -> bytes:
        return struct.pack("<BIIIIB", self.header, self.firmware_size, self.package_size, self.buffer_size, self.compressed_size, self.flags.to_byte())

    @classmethod
    def from_bytes(cls, data: bytes):
        _, firmware_size, package_size, buffer_size, compressed_size, flags_byte = struct.unpack("<BIIIIB", data)
        flags = BeginReq.Flags.from_byte(flags_byte)
        return cls(firmware_size, package_size, buffer_size, compressed_size, flags)


@dataclass
class BeginResp(Message):
    package_size: int
    buffer_size: int

    def __init__(self, package_size, buffer_size):
        super().__init__(HeaderCode.BEGIN_RESP)
        self.package_size = package_size
        self.buffer_size = buffer_size

    def to_bytes(self) -> bytes:
        return struct.pack("<BII", self.header, self.package_size, self.buffer_size)

    @classmethod
    def from_bytes(cls, data: bytes):
        _, pkg_size, buf_size = struct.unpack("<BII", data)
        return cls(pkg_size, buf_size)


@dataclass
class Package(Message):
    data: bytes

    def __init__(self, header: HeaderCode, data: bytes):
        super().__init__(header)
        self.data = data

    def to_bytes(self) -> bytes:
        return struct.pack("<B", self.header) + self.data

    @classmethod
    def from_bytes(cls, data: bytes):
        header = HeaderCode(data[0])
        return cls(header, data[1:])


@dataclass
class PackageInd(Package):
    def __init__(self, data: bytes):
        super().__init__(HeaderCode.PACKAGE_IND, data)


@dataclass
class PackageReq(Package):
    def __init__(self, data: bytes):
        super().__init__(HeaderCode.PACKAGE_REQ, data)


@dataclass
class PackageResp(Message):
    def __init__(self):
        super().__init__(HeaderCode.PACKAGE_RESP)


@dataclass
class EndReq(Message):
    firmware_crc: int

    def __init__(self, firmware_crc):
        super().__init__(HeaderCode.END_REQ)
        self.firmware_crc = firmware_crc

    def to_bytes(self) -> bytes:
        return struct.pack("<BI", self.header, self.firmware_crc)

    @classmethod
    def from_bytes(cls, data: bytes):
        _, crc = struct.unpack("<BI", data)
        return cls(crc)


@dataclass
class EndResp(Message):
    def __init__(self):
        super().__init__(HeaderCode.END_RESP)


@dataclass
class ErrorInd(Message):
    code: ErrorCode

    def __init__(self, code: ErrorCode):
        super().__init__(HeaderCode.ERROR_IND)
        self.code = code

    def to_bytes(self) -> bytes:
        return struct.pack("<BB", self.header, self.code)

    @classmethod
    def from_bytes(cls, data: bytes):
        _, code = struct.unpack("<BB", data)
        return cls(ErrorCode(code))


@dataclass
class UploadEnableInd(Message):
    def __init__(self):
        super().__init__(HeaderCode.UPLOAD_ENABLE_IND)


@dataclass
class UploadDisableInd(Message):
    def __init__(self):
        super().__init__(HeaderCode.UPLOAD_DISABLE_IND)


@dataclass
class SetPinReq(Message):
    pin: int

    def __init__(self, pin: int):
        super().__init__(HeaderCode.SET_PIN_REQ)
        self.pin = pin

    def to_bytes(self) -> bytes:
        return struct.pack("<BI", self.header, self.pin)

    @classmethod
    def from_bytes(cls, data: bytes):
        _, pin = struct.unpack("<BI", data)
        return cls(pin)


@dataclass
class SetPinResp(Message):
    def __init__(self):
        super().__init__(HeaderCode.SET_PIN_RESP)


@dataclass
class RemovePinReq(Message):
    def __init__(self):
        super().__init__(HeaderCode.REMOVE_PIN_REQ)


@dataclass
class RemovePinResp(Message):
    def __init__(self):
        super().__init__(HeaderCode.REMOVE_PIN_RESP)


@dataclass
class SignatureReq(Package):
    def __init__(self, data: bytes):
        super().__init__(HeaderCode.SIGNATURE_REQ, data)


@dataclass
class SignatureResp(Message):
    def __init__(self):
        super().__init__(HeaderCode.SIGNATURE_RESP)


HEADER_TO_MESSAGE = {
    HeaderCode.INIT_REQ: InitReq,
    HeaderCode.INIT_RESP: InitResp,
    HeaderCode.BEGIN_REQ: BeginReq,
    HeaderCode.BEGIN_RESP: BeginResp,
    HeaderCode.PACKAGE_IND: PackageInd,
    HeaderCode.PACKAGE_REQ: PackageReq,
    HeaderCode.PACKAGE_RESP: PackageResp,
    HeaderCode.END_REQ: EndReq,
    HeaderCode.END_RESP: EndResp,
    HeaderCode.ERROR_IND: ErrorInd,
    HeaderCode.UPLOAD_ENABLE_IND: UploadEnableInd,
    HeaderCode.UPLOAD_DISABLE_IND: UploadDisableInd,
    HeaderCode.SET_PIN_REQ: SetPinReq,
    HeaderCode.SET_PIN_RESP: SetPinResp,
    HeaderCode.REMOVE_PIN_REQ: RemovePinReq,
    HeaderCode.REMOVE_PIN_RESP: RemovePinResp,
    HeaderCode.SIGNATURE_REQ: SignatureReq,
    HeaderCode.SIGNATURE_RESP: SignatureResp,
}


T = TypeVar("T", bound=Message)


def parse_message_of_type(data: bytes, expected_cls: Type[T]) -> T:
    header = HeaderCode(data[0])
    cls = HEADER_TO_MESSAGE.get(header)
    if cls is None:
        raise ValueError(f"Unknown message header: {header}")

    msg = cls.from_bytes(data)

    if isinstance(msg, ErrorInd):
        raise ValueError(ERROR_TO_STR[msg.code])

    if not isinstance(msg, expected_cls):
        raise ValueError(f"Message type mismatch: expected {expected_cls.__name__}, got {type(msg).__name__}")

    return msg


ERROR_TO_STR = {
    ErrorCode.OK: "OK",
    ErrorCode.NOK: "Not ok",
    ErrorCode.INCORRECT_FORMAT: "Incorrect format",
    ErrorCode.INCORRECT_FIRMWARE_SIZE: "Incorrect firmware size",
    ErrorCode.INTERNAL_STORAGE_ERROR: "Internal storage error",
    ErrorCode.UPLOAD_DISABLED: "Upload disabled",
    ErrorCode.UPLOAD_RUNNING: "Upload running",
    ErrorCode.UPLOAD_STOPPED: "Upload stopped",
    ErrorCode.INSTALL_RUNNING: "Install running",
    ErrorCode.BUFFER_DISABLED: "Buffer disabled",
    ErrorCode.BUFFER_OVERFLOW: "Buffer overflow",
    ErrorCode.COMPRESSION_NOT_SUPPORTED: "Compression not supported",
    ErrorCode.INCORRECT_COMPRESSION: "Incorrect compression",
    ErrorCode.INCORRECT_COMPRESSED_SIZE: "Incorrect compressed size",
    ErrorCode.INCORRECT_COMPRESSION_CHECKSUM: "Incorrect compression checksum",
    ErrorCode.INCORRECT_COMPRESSION_PARAM: "Incorrect compression param",
    ErrorCode.INCORRECT_COMPRESSION_END: "Incorrect compression end",
    ErrorCode.CHECKSUM_NOT_SUPPORTED: "Checksum not supported",
    ErrorCode.INCORRECT_CHECKSUM: "Incorrect checksum",
    ErrorCode.SIGNATURE_NOT_SUPPORTED: "Signature not supported",
    ErrorCode.INCORRECT_SIGNATURE: "Incorrect signature",
    ErrorCode.INCORRECT_SIGNATURE_SIZE: "Incorrect signature size",
}
