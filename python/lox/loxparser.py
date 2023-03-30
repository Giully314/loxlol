from dataclasses import dataclass, field 
from loxtoken import Token, TokenType
import expression as expr
from typing import Union
import error_reporter as err




@dataclass
class Parser:
    """
    A recursive descent parser. 
    The grammar on which operates:
    expression -> equality ;
    equality   -> comparison ( ("!=" | "==") comparison)* ;
    comparison -> term ( (">" | ">=" | "<" | "<=") term)* ;
    term       -> factor ( ("-" | "+") factor)* ;
    factor     -> unary ( ("*" | "/") unary)* ;
    unary      -> ("!" | "-") unary | primary ;
    primary    -> NUMBER | STRING | "true" | "false" | "nil" | "(" expression ")" ;   
    """

    tokens: list[Token]
    current: int = field(default=0, init=False)


    class ParseError(Exception):
        """
        Used to detect when to synchronize after a parse error. 
        """
        ...


    def parse(self) -> expr.Expression:
        try:
            return self.__expression()
        except self.ParseError as e:
            return None


    def __expression(self) -> expr.Expression:
        return self.__equality()
    

    def __equality(self) -> Union[expr.Expression, expr.Binary]:
        expression = self.__comparison()

        while self.__match(TokenType.BANG_EQUAL, TokenType.EQUAL_EQUAL):
            operator = self.__previous()
            right = self.__comparison()
            expression = expr.Binary(operator, expression, right)
        
        return expression


    def __comparison(self) -> Union[expr.Expression, expr.Binary]:
        expression = self.__term()
        
        while self.__match(TokenType.GREATER, TokenType.GREATER_EQUAL, TokenType.LESS, TokenType.LESS_EQUAL):
            operator = self.__previous()
            right = self.__term()
            expression = expr.Binary(operator, expression, right)
        
        return expression

    def __term(self) -> Union[expr.Expression, expr.Binary]:
        expression = self.__factor()
        
        while self.__match(TokenType.MINUS, TokenType.PLUS):
            operator = self.__previous()
            right = self.__factor()
            expression = expr.Binary(operator, expression, right)
        
        return expression


    def __factor(self) -> Union[expr.Expression, expr.Unary, expr.Binary]:
        expression = self.__unary()
        
        while self.__match(TokenType.STAR, TokenType.SLASH):
            operator = self.__previous()
            right = self.__unary()
            expression = expr.Binary(operator, expression, right)
        
        return expression


    def __unary(self) -> Union[expr.Expression, expr.Unary]:
        if self.__match(TokenType.BANG, TokenType.MINUS):
            operator = self.__previous()
            right = self.__unary()
            return expr.Unary(operator, right)

        return self.__primary()
    

    def __primary(self) -> expr.Expression:
        if self.__match(TokenType.FALSE):
            return expr.Literal(False)
        if self.__match(TokenType.TRUE):
            return expr.Literal(True)
        if self.__match(TokenType.NIL):
            return expr.Literal(None)

        if self.__match(TokenType.NUMBER, TokenType.STRING):
            return expr.Literal(self.__previous().literal)
        
        if self.__match(TokenType.LEFT_PAREN):
            expression = self.__expression()
            self.__consume(TokenType.RIGHT_PAREN, "Expect ')' after expression.")
            return expr.Grouping(expression)
        
        raise self.__error(self.__peek(), "Expect expression.")
        

    def __consume(self, token_type: TokenType, msg: str) -> Token: 
        if self.__check(token_type):
            return self.__advance()
        raise self.__error(self.__peek(), msg)
    
    def __error(self, token: Token, msg: str) -> ParseError:
        err.error_reporter.error_token(token, msg)
        return self.ParseError()

    def __match(self, *token_types: TokenType) -> bool:
        for token_type in token_types:
            if self.__check(token_type):
                self.__advance()
                return True
        return False

    def __check(self, token_type: TokenType) -> bool:
        if self.__is_at_end():
            return False
        return self.__peek().type == token_type


    def __synchronize(self):
        self.__advance()
        while not self.__is_at_end():
            if self.__previous().type == TokenType.SEMICOLON:
                return
            
            match self.__peek().type:
                case TokenType.CLASS | TokenType.FUN | TokenType.VAR | TokenType.FOR | TokenType.IF | \
                        TokenType.WHILE | TokenType.PRINT | TokenType.RETURN:
                    return
            
            self.__advance()


    def __advance(self):
        if not self.__is_at_end():
            self.current += 1
        return self.__previous()    

    def __peek(self) -> TokenType:
        return self.tokens[self.current]

    def __previous(self) -> Token:
        return self.tokens[self.current - 1]

    def __is_at_end(self):
        return self.__peek().type == TokenType.EOF