from loxtoken import Token, TokenType
import expression as expr
import visitor

class Interpreter:
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
                return left * right
            case TokenType.SLASH:
                return left / right
            case TokenType.MINUS:
                return left - right
            case TokenType.PLUS:
                # NOTE: this distinction in python is useless because of the dynamic type system.
                # So we can just return left + right independently of the type.
                # In the book the checks on the types is done because java uses a static type system. 
                
                # Plus can be use to sum 2 numbers or
                # if isinstance(left, float) and isinstance(right, float):
                #     return left + right
                # # concatenate 2 strings
                # elif isinstance(left, str) and isinstance(right, str):
                #     return left + right
                return left + right 
            
            # comparison operators
            case TokenType.GREATER:
                return left > right
            case TokenType.GREATER_EQUAL:
                return left >= right
            case TokenType.LESS:
                return left < right
            case TokenType.LESS_EQUAL:
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