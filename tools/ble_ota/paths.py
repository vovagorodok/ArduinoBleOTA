from dataclasses import dataclass

@dataclass
class Paths():
    firmware: str
    private_key: str | None

    def __init__(self, firmware_path: str, private_key: str = None):
        self.firmware = firmware_path
        self.private_key = private_key

    @classmethod
    def parse(cls, argv: list[str]):
        return cls(argv[1], argv[2] if len(argv) > 2 else None)
