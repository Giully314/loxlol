from loxtoken import Token, TokenType
import expression as expr
import visitor
from error_reporter import error_reporter
from loxerror import LoxRuntimeError, LoxNonInitializedVar
import statement as stmt
from dataclasses import dataclass, field
from environment import Environment

@dataclass
class Interpreter:
    env: Environment = field(default_factory=Environment, init=False)

    def interpret(self, statements: list[stmt.Statement]) -> str:
        """
        Can raise a LoxRuntimeError.
        """
        try:
            for statement in statements:
                self.visit(statement)
        except LoxRuntimeError as err:
            error_reporter.runtime_error(err)

    def __stringify(self, obj: object) -> str:
        if obj is None:
            return "nil"
        return str(obj)
    

    def __execute_block(self, s: stmt.Block, env: Environment):
        # Save the current environment for later, after the block finish its execution.
        previous_env = self.env

        try:
            self.env = env

            for statement in s.statements:
                self.visit(statement)
        finally:
            self.env = previous_env


    @visitor.visitor(stmt.Block)
    def visit(self, s: stmt.Block):
        self.__execute_block(s, Environment(self.env))


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
        return self.env[e.name]


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