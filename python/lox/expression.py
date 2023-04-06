from loxtoken import Token
from abc import ABC
from dataclasses import dataclass

class Expression(ABC):
    ...

@dataclass(frozen=True)
class Unary(Expression):
    operator: Token
    right: Expression


@dataclass(frozen=True)
class Grouping(Expression):
    expr: Expression


@dataclass(frozen=True)
class Literal(Expression):
    value: object


@dataclass(frozen=True)
class Binary(Expression):
    operator: Token
    left: Expression
    right: Expression


@dataclass(frozen=True)
class Variable(Expression):
    name: Token


@dataclass(frozen=True)
class Assign(Expression):
    name: Token
    value: Expression


