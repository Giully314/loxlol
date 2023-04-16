from loxtoken import Token, TokenType
import expression as expr
import visitor
from error_reporter import error_reporter
from loxerror import LoxRuntimeError, LoxNonInitializedVar, LoxReturn
import statement as stmt
from dataclasses import dataclass, field
from environment import Environment
from loxtypes import LoxCallable, LoxFunction
import time

@dataclass
class Interpreter:
    global_env: Environment = field(default_factory=Environment, init=False)
    env: Environment = None
    locals: dict[expr.Expression, int] = field(default_factory=dict, init=False)

    def __post_init__(self):
        self.env = self.global_env
        
        class Clock(LoxCallable):
            def call(self, interpreter, *args: object):
                return time.time()

            def arity(self) -> int:
                return 0

            def __str__(self) -> str:
                return "<native fn>"
            
        self.global_env.define_name("clock", Clock())


    def interpret(self, statements: list[stmt.Statement]) -> str:
        """
        Can raise a LoxRuntimeError.
        """
        try:
            for statement in statements:
                self.visit(statement)
        except LoxRuntimeError as err:
            error_reporter.runtime_error(err)
    

    @visitor.visitor(stmt.Return)
    def visit(self, s: stmt.Return):
        value = None
        if s.value is not None:
            value = self.visit(s.value)
        raise LoxReturn(value)

    
    @visitor.visitor(stmt.Function)
    def visit(self, s: stmt.Function):
        function = LoxFunction(s, self.env)
        self.env.define(s.name, function)


    @visitor.visitor(stmt.Block)
    def visit(self, s: stmt.Block):
        self._execute_block(s.statements, Environment(self.env))


    @visitor.visitor(stmt.Var)
    def visit(self, s: stmt.Var):
        obj = LoxNonInitializedVar()
        if s.initializer is not None:
            obj = self.visit(s.initializer)
        self.env.define(s.name, obj)
    

    @visitor.visitor(stmt.StmtExpression)
    def visit(self, s: stmt.StmtExpression):
        self.visit(s.expr)


    @visitor.visitor(stmt.Print)
    def visit(self, s: stmt.Print):
        obj = self.visit(s.expr)
        print(self.__stringify(obj))


    @visitor.visitor(stmt.If)
    def visit(self, s: stmt.If):
        if self.__is_truthful(self.visit(s.condition)):
            self.visit(s.then_branch)
        elif s.else_branch is not None:
            self.visit(s.else_branch)

    
    @visitor.visitor(stmt.While)
    def visit(self, s: stmt.While):
        while self.visit(s.condition):
            self.visit(s.body)


    @visitor.visitor(expr.Call)
    def visit(self, e: expr.Call) -> object:
        callee = self.visit(e.callee)

        if not isinstance(callee, LoxCallable):
            raise LoxRuntimeError("Can only call functions and classes." ,e.paren)
        
        arguments = []
        for argument in e.arguments:
            arguments.append(self.visit(argument))

        if callee.arity() != len(arguments):
            raise LoxRuntimeError(f"Expected {callee.arity()} arguments but got {len(arguments)}.", e.paren)

        return callee.call(self, arguments)

    @visitor.visitor(expr.Logical)
    def visit(self, e: expr.Logical) -> object:
        left = self.visit(e.left)

        if e.operator.type == TokenType.OR:
            if self.__is_truthful(left):
                return left 
        if e.operator.type == TokenType.AND:
            if not self.__is_truthful(left):
                return left

        return self.visit(e.right)
    

    @visitor.visitor(expr.Assign)
    def visit(self, e: expr.Assign) -> object:
        value = self.visit(e.value)
        self.env.assign(e.name, value)
        return value


    # TODO: check if the executed branch is an expression, that is, must return a value.
    # The conditional operator ?: is only supported for expressions and not for statements.
    @visitor.visitor(expr.Conditional)
    def visit(self, e: expr.Conditional) -> object:
        if self.__is_truthful(self.visit(e.condition)):
            return self.visit(e.then_branch)
        elif e.else_branch is not None:
            return self.visit(e.else_branch)


    @visitor.visitor(expr.Variable)
    def visit(self, e: expr.Variable):
        return self.__lookup_variable(e.name, e)


    @visitor.visitor(expr.Literal)
    def visit(self, e: expr.Literal) -> object:
        return e.value
    

    @visitor.visitor(expr.Grouping)
    def visit(self, e: expr.Grouping) -> object:
        return self.visit(e.expr)
    
    
    @visitor.visitor(expr.Unary)
    def visit(self, e: expr.Unary) -> object:
        right = self.visit(e.right)

        match e.operator.type:
            case TokenType.MINUS:
                self.__check_number_operand(e.operator, right)
                return -right
            case TokenType.BANG:
                return not self.__is_truthful(right)
        
        # This is unreachable.
        return None

    
    @visitor.visitor(expr.Binary)
    def visit(self, e: expr.Binary) -> object:
        left = self.visit(e.left)
        right = self.visit(e.right)

        match e.operator.type: 

            # arithmetic operators (and string concatenation)
            case TokenType.STAR:
                self.__check_number_operands(e.operator, left, right)
                return left * right
            case TokenType.SLASH:
                self.__check_number_operands(e.operator, left, right)
                self.__check_operand_zero(e.operator, right)
                return left / right
            case TokenType.MINUS:
                self.__check_number_operands(e.operator, left, right)
                return left - right
            case TokenType.PLUS:
                # TODO: this is meh
                if isinstance(left, str) or isinstance(right, str):
                    return str(left) + str(right)
                self.__check_number_operands(e.operator, left, right)
                return left + right
            
            # comparison operators
            case TokenType.GREATER:
                self.__check_number_string_operands(e.operator, left, right)
                return left > right
            case TokenType.GREATER_EQUAL:
                self.__check_number_string_operands(e.operator, left, right)
                return left >= right
            case TokenType.LESS:
                self.__check_number_string_operands(e.operator, left, right)
                return left < right
            case TokenType.LESS_EQUAL:
                self.__check_number_string_operands(e.operator, left, right)
                return left <= right
            case TokenType.EQUAL_EQUAL:
                return left == right
            case TokenType.BANG_EQUAL:
                return left != right
    
        # This is unreachable.
        return None
    

    def __lookup_variable(self, name: Token, e: expr.Expression):
        distance = self.locals.get(e, None)
        if distance is not None:
            return self.env.get_at(distance, name.lexeme)
        else:
            return self.global_env[name]
    

    def _resolve(self, e: expr.Expression, depth: int):
        self.locals[e] = depth


    def __stringify(self, obj: object) -> str:
        if obj is None:
            return "nil"
        return str(obj)
    
    
    def _execute_block(self, statements: list[stmt.Statement], env: Environment):
        # Save the current environment for later, after the block finish its execution.
        previous_env = self.env
        try:
            self.env = env

            for statement in statements:
                self.visit(statement)
        finally:
            self.env = previous_env
    
    
    def __is_truthful(self, obj: object) -> bool:
        if obj is None:
            return False
        if isinstance(obj, bool):
            return obj
        return True
    
    def __check_number_operand(self, operator: Token, operand: object):
        if isinstance(operand, float):
            return
        raise LoxRuntimeError("Operand must be a number.", operator)
    
    def __check_number_operands(self, operator: Token, left: object, right: object):
        if isinstance(left, float) and isinstance(right, float):
            return
        raise LoxRuntimeError("Operands must be two numbers.", operator)
    
    def __check_number_string_operands(self, operator: Token, left: object, right: object):
        """
        Check if the two operands are both numbers OR both strings.
        """
        if (isinstance(left, float) and isinstance(right, float)) or \
            (isinstance(left, str) and isinstance(right, str)):
            return
        raise LoxRuntimeError("Operands must be two numbers or two strings.", operator)
    

    def __check_operand_zero(self, operator: Token, divisor: float):
        if divisor != 0:
            return
        raise LoxRuntimeError("Divisor must not be zero.", operator)