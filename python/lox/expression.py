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


@dataclass(frozen=True)
class Conditional(Expression):
    condition: Expression
    then_branch: Expression
    else_branch: Expression


@dataclass(frozen=True)
class Logical(Expression):
    operator: Token
    left: Expression
    right: Expression


@dataclass(frozen=True)
class Call(Expression):
    callee: Expression
    paren: Token
    arguments: list[Expression]
