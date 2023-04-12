from loxtoken import Token
from dataclasses import dataclass

class LoxRuntimeError(RuntimeError):
    def __init__(self, msg: str, token: Token):
        super().__init__(msg)
        self.token = token

class LoxNonInitializedVar:
    ...

@dataclass(frozen=True)
class LoxReturn(Exception):
    value: object  