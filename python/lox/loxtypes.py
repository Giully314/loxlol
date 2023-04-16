from abc import ABC, abstractmethod
from environment import Environment
from dataclasses import dataclass
from statement import Function
from loxerror import LoxReturn
from typing import Optional

class LoxCallable(ABC):
    
    @abstractmethod
    def call(self, interpreter, *args: object):
        ...

    @abstractmethod
    def arity(self) -> int:
        ...


@dataclass
class LoxFunction(LoxCallable):
    declaration: Function
    closure: Environment

    def call(self, interpreter, args: list[object]) -> Optional[object]:
        fun_env = Environment(self.closure)

        for i, arg in enumerate(args):
            fun_env.define(self.declaration.params[i], arg)

        try:
            interpreter._execute_block(self.declaration.body, fun_env)
        except LoxReturn as r:
            return r.value
        


    def arity(self) -> int:
        return len(self.declaration.params)
    

    def __str__(self) -> str:
        return f"<fn {self.declaration.name.lexeme}>"
    
