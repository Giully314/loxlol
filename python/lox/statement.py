from loxtoken import Token
from abc import ABC
from dataclasses import dataclass
from expression import Expression

class Statement(ABC):
    ...

@dataclass(frozen=True)
class StmtExpression(Statement):
    expr: Expression


@dataclass(frozen=True)
class Print(Statement):
    expr: Expression


@dataclass(frozen=True)
class Var(Statement):
    name: Token
    initializer: Expression


@dataclass(frozen=True)
class Block(Statement):
    statements: list[Statement]


@dataclass(frozen=True)
class If(Statement):
    condition: Expression
    then_branch: Expression
    else_branch: Expression


@dataclass(frozen=True)
class While(Statement):
    condition: Expression = None
    body: Statement = None
    should_break: bool = False


@dataclass(frozen=True)
class Break(Statement):
    keyword: Token


@dataclass(frozen=True)
class Function:
    name: Token
    params: list[Token]
    body: list[Statement]


@dataclass(frozen=True)
class Return:
    keyword: Token
    value: Expression