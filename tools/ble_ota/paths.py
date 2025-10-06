from typing import NamedTuple

class InputPaths(NamedTuple):
    firmware: str
    private_key: str | None = None

    @classmethod
    def parse(cls, argv: list[str]):
        return cls(argv[1], argv[2] if len(argv) > 2 else None)
