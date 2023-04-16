from __future__ import annotations
from dataclasses import dataclass, field
from loxerror import LoxRuntimeError, LoxNonInitializedVar
from loxtoken import Token 


@dataclass
class Environment:
    outer_env: Environment = None
    variables: dict[str, object] = field(default_factory=dict, init=False)

    def get_at(self, distance: int, name: str) -> object:
        return self.__ancestor(distance).variables[name]


    def define_name(self, name: str, value: object):
        self.variables[name] = value

    def define(self, name: Token, value: object):
        self.variables[name.lexeme] = value

    def assign(self, name: Token, value: object):
        if name.lexeme in self.variables:
           self.variables[name.lexeme] = value
           return
        
        if self.outer_env is not None:
            self.outer_env.assign(name, value)
            return

        raise LoxRuntimeError(f"Undefined variable '{name.lexeme}'.", name)

    def __getitem__(self, name: Token) -> object:
        if name.lexeme in self.variables:
            if isinstance(self.variables[name.lexeme], LoxNonInitializedVar):
                raise LoxRuntimeError(f"{name.lexeme} is not initialized.", name)
            return self.variables[name.lexeme]
        
        if self.outer_env is not None:
            return self.outer_env[name]

        raise LoxRuntimeError(f"Undefined variable '{name.lexeme}'.", name)
    
    def __setitem__(self, name: Token, value: object):
        """Calls self.assign(name, value)"""
        self.assign(name, value)



    def __ancestor(self, distance: int) -> Environment:
        env = self
        for i in range(distance):
            env = env.outer_env
        return env