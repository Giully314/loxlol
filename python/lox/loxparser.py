from dataclasses import dataclass, field 
from loxtoken import Token, TokenType
import expression as expr
import statement as stmt
from typing import Union
import error_reporter as err


@dataclass
class Parser:
    """
    A recursive descent parser.
    """

    tokens: list[Token]
    current: int = field(default=0, init=False)


    class ParseError(Exception):
        """
        Used to detect when to synchronize after a parse error. 
        """
        ...


    def parse(self) -> list[stmt.Statement]:
        """
        This method corresponds to "program" in the grammar.
        """
        statements = []
        while not self.__is_at_end():
            statements.append(self.__declaration())

        return statements

    def __declaration(self) -> stmt.Statement:
        try:
            if self.__match(TokenType.VAR):
                return self.__var_declaration()
            return self.__statement()
        except Parser.ParseError as e:
            self.__synchronize()
            return None
        
    def __var_declaration(self) -> stmt.Statement:
        name = self.__consume(TokenType.IDENTIFIER, "Expect a variable name.")
        initializer: expr.Expression = None
        
        if self.__match(TokenType.EQUAL):
            initializer = self.__expression()
        
        self.__consume(TokenType.SEMICOLON, "Expect ';' after variable declaration.")
        return stmt.Var(name, initializer)

    def __statement(self) -> Union[stmt.If, stmt.Print, list[stmt.Statement], stmt.StmtExpression]:
        if self.__match(TokenType.IF):
            return self.__if_stmt()

        if self.__match(TokenType.PRINT):
            return self.__print_stmt()
        
        if self.__match(TokenType.LEFT_BRACE):
            return stmt.Block(self.__block())
        
        return self.__expression_stmt()
    

    def __if_stmt(self) -> stmt.If:
        self.__consume(TokenType.LEFT_PAREN, "Expect '(' after 'if'.")
        condition = self.__expression()
        self.__consume(TokenType.RIGHT_PAREN, "Expect ')' after if condition.")

        then_branch = self.__statement()
        else_branch = None
        if self.__match(TokenType.ELSE):
            else_branch = self.__statement()

        return stmt.If(condition, then_branch, else_branch)    


    def __block(self) -> list[stmt.Statement]:
        statements: list[stmt.Statement] = []

        while not (self.__is_at_end() or self.__check(TokenType.RIGHT_BRACE)):
            statements.append(self.__declaration())

        self.__consume(TokenType.RIGHT_BRACE, "Expect '}' after block.")

        return statements


    def __print_stmt(self) -> stmt.Print:
        expression = self.__expression()
        self.__consume(TokenType.SEMICOLON, "Expect ';' after expression.")
        return stmt.Print(expression)
    
    
    def __expression_stmt(self) -> stmt.StmtExpression:
        expression = self.__expression()
        self.__consume(TokenType.SEMICOLON, "Expect ';' after expression.")
        return stmt.StmtExpression(expression)


    def __expression(self) -> expr.Expression:
        return self.__comma()
    

    def __assignment(self) -> expr.Expression:
        expression = self.__ternary()

        if self.__match(TokenType.EQUAL):
            equals = self.__previous()
            value = self.__assignment()

            if isinstance(expression, expr.Variable):
                name = expression.name
                return expr.Assign(name, value)
            
            # if is not a valid assignment report the error. No need to raise it because the parser 
            # isn't in a confused state so there isn't the necessity to synchronize.
            err.error_reporter.error_token(equals, "Invalid assignment target.")

        return expression
        
    # Exercise 6.1
    def __comma(self) -> expr.Expression:
        expression = self.__assignment()

        while self.__match(TokenType.COMMA):
            operator = self.__previous()
            right = self.__assignment()
            expression = expr.Binary(operator, expression, right)
        
        return expression 
    
    # Exercise 6.2
    # TODO: not sure if it is correct for multiple nested cases.
    # For single expr ? value1 : value2; it works.
    def __ternary(self) -> expr.Expression:
        expression = self.__logical_or()

        while self.__match(TokenType.QUESTION):
            operator = self.__previous()
            condition = expression
            then_branch = self.__logical_or()
            else_branch = self.__logical_or()
            expression = expr.Conditional(condition, then_branch, else_branch)

        return expression

    def __logical_or(self) -> expr.Expression:
        expression = self.__logical_and()

        while self.__match(TokenType.OR):
            operator = self.__previous()
            right = self.__logical_and()
            expression = expr.Logical(operator, expression, right)

        return expression

    def __logical_and(self) -> expr.Expression:
        expression = self.__equality()

        while self.__match(TokenType.AND):
            operator = self.__previous()
            right = self.__equality()
            expression = expr.Logical(operator, expression, right)

        return expression

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
        
        if self.__match(TokenType.COMMA):
            expression = self.__expression()
            self.__consume(TokenType.COMMA, "Missing comma for multiple expression in the same statement")
            return expression
        
        if self.__match(TokenType.IDENTIFIER):
            return expr.Variable(self.__previous())
        
        # if self.__match(TokenType.QUESTION):
        #     expression = self.__expression()
        #     self.__consume(TokenType.QUESTION, )
        
        raise self.__error(self.__peek(), f"Expect expression at {self.current}.")
        

    def __consume(self, token_type: TokenType, msg: str) -> Token: 
        """
        Consume a token while checking if it is of the type passed as argument.
        """
        if self.__check(token_type):
            return self.__advance()
        raise self.__error(self.__peek(), msg)
    
    def __error(self, token: Token, msg: str) -> ParseError:
        """
        Report the error related to the token passed as argument and raise a ParseError.
        """
        err.error_reporter.error_token(token, msg)
        return self.ParseError()

    def __match(self, *token_types: TokenType) -> bool:
        """
        Check if the current token is one of the types passed.
        """
        for token_type in token_types:
            if self.__check(token_type):
                self.__advance()
                return True
        return False

    def __check(self, token_type: TokenType) -> bool:
        """
        Check if the current token has the same type as token_type.
        """
        if self.__is_at_end():
            return False
        return self.__peek().type == token_type


    def __synchronize(self):
        """
        Discard all the tokens that are inside an expression that raised an error and put the parser
        in a state in which can continue to operate.
        """
        self.__advance()
        while not self.__is_at_end():
            if self.__previous().type == TokenType.SEMICOLON:
                return
            
            match self.__peek().type:
                case TokenType.CLASS | TokenType.FUN | TokenType.VAR | TokenType.FOR | TokenType.IF | \
                        TokenType.WHILE | TokenType.PRINT | TokenType.RETURN:
                    return
            
            self.__advance()


    def __advance(self) -> Token:
        if not self.__is_at_end():
            self.current += 1
        return self.__previous()    

    def __peek(self) -> TokenType:
        return self.tokens[self.current]

    def __previous(self) -> Token:
        return self.tokens[self.current - 1]

    def __is_at_end(self) -> bool:
        return self.__peek().type == TokenType.EOF