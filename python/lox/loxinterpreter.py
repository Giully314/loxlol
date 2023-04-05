from loxtoken import Token, TokenType
import expression as expr
import visitor
from error_reporter import error_reporter
from loxerror import LoxRuntimeError


class Interpreter:


    def interpret(self, e: expr.Expression) -> str:
        """
        Can raise a LoxRuntimeError.
        """
        try:
            result = self.visit(e)
            print(self.__stringify(result))
        except LoxRuntimeError as err:
            error_reporter.runtime_error(err)

    def __stringify(self, obj: object) -> str:
        if obj is None:
            return "nil"
        return str(obj)


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