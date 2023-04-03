import loxtoken as t
from abc import ABC
from dataclasses import dataclass

class Expression(ABC):
    ...

@dataclass(frozen=True)
class Unary(Expression):
    operator: t.Token
    right: Expression


@dataclass(frozen=True)
class Grouping(Expression):
    expr: Expression


@dataclass(frozen=True)
class Literal(Expression):
    value: object


@dataclass(frozen=True)
class Binary(Expression):
    operator: t.Token
    left: Expression
    right: Expression



@dataclass(frozen=True)
class Conditional(Expression):
    condition: Expression
    then_branch: Expression
    else_branch: Expression


