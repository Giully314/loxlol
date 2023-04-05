from loxtoken import Token

class LoxRuntimeError(RuntimeError):
    def __init__(self, msg: str, token: Token):
        super().__init__(msg)
        self.token = token
