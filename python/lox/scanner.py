from dataclasses import dataclass, field
from enum import Enum
from error_reporter import error_reporter
from loxtoken import Token, TokenType


@dataclass 
class Scanner:
    source_code: str
    start: int = field(default=0, init=False)
    current: int = field(default=0, init=False)
    line: int = field(default=1, init=False)
    tokens: list[Token] = field(default_factory=list, init=False)
    reserved_keywords: dict[str, TokenType] = field(default_factory=dict, init=False)

    def __post_init__(self):
        self.reserved_keywords = {
            "and"       :   TokenType.AND,
            "class"     :   TokenType.CLASS,
            "else"      :   TokenType.ELSE,
            "false"     :  	TokenType.FALSE,
            "for"       :   TokenType.FOR,
            "fun"       :   TokenType.FUN,
            "if"        :   TokenType.IF,
            "nil"       :   TokenType.NIL,
            "or"        :   TokenType.OR,
            "print"     :  	TokenType.PRINT,
            "return"    : 	TokenType.RETURN,
            "super"     :  	TokenType.SUPER,
            "this"      :   TokenType.THIS,
            "true"      :   TokenType.TRUE,
            "var"       :   TokenType.VAR,
            "while"     :  	TokenType.WHILE,
        }


    def scan_tokens(self) -> list[Token]:
        while not self.__is_at_end():
            self.start = self.current
            self.scan_token()

        self.tokens.append(Token(TokenType.EOF, "", None, self.line))
        return self.tokens


    def scan_token(self):
        c = self.__advance()

        match c:
            case '(': 
                self.__add_token(TokenType.LEFT_PAREN) 
            case ')': 
                self.__add_token(TokenType.RIGHT_PAREN)
            case '{': 
                self.__add_token(TokenType.LEFT_BRACE) 
            case '}': 
                self.__add_token(TokenType.RIGHT_BRACE) 
            case ',': 
                self.__add_token(TokenType.COMMA) 
            case '.': 
                self.__add_token(TokenType.DOT) 
            case '-': 
                self.__add_token(TokenType.MINUS) 
            case '+': 
                self.__add_token(TokenType.PLUS) 
            case ';': 
                self.__add_token(TokenType.SEMICOLON) 
            case '*': 
                self.__add_token(TokenType.STAR) 
            
            # comparison
            case '!':
                self.__add_token(TokenType.BANG_EQUAL if self.__match_char('=') else TokenType.BANG)
            case '=':
                self.__add_token(TokenType.EQUAL_EQUAL if self.__match_char('=') else TokenType.EQUAL)
            case '<':
                self.__add_token(TokenType.LESS_EQUAL if self.__match_char('=') else TokenType.LESS)
            case '>':
                self.__add_token(TokenType.GREATER_EQUAL if self.__match_char('=') else TokenType.GREATER)
            

            case '/': 
                if self.__match_char('/'): # check if this is the division operator or the start of a comment
                # A comment goes until the end of the line.
                    while self.__peek() != '\n' and not self.__is_at_end(): 
                        self.__advance()
                elif self.__match_char('*'): # not sure if it is correct.
                    # /* bla bla bla */
                    good_comment = False
                    while not self.__is_at_end():
                        if self.__peek() == "*" and self.__peek_next() == "/":
                            self.__advance()
                            self.__advance()
                            good_comment = True
                            break
                        
                        if self.__peek() == "\n":
                            self.line += 1
                        
                        print(f"{self.__peek()}")
                        self.__advance()
                    
                    if not good_comment and self.__is_at_end():
                        error_reporter.error(self.line, "Missing */ for closing the comment.")
                else:
                    self.__add_token(TokenType.SLASH)

            # Ternary operator
            case '?':
                self.__ternary()

            case ':':
                ...

            # whitespaces are ignored.
            case ' ':
                ...
            case '\r':
                ... 
            case '\t':
                ... 

            case '\n':
                self.line += 1

            # literals 
            case '"':
                self.__string()

            case "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9":
                self.__number()

            # default case is to check if a the current char is a letter or if it not recognized
            case _:
                if c.isalpha():
                    self.__identifier() 
                else:
                    error_reporter.error(self.line, f"Unexpected character {c}.")

    def __ternary(self):
        self.__add_token(TokenType.QUESTION)
        self.start = self.current
        while not self.__match_char(":"):
            if self.__is_at_end():
                error_reporter.error(self.line, f"Expected ':' in ternary operator '?:' at {self.current}")
                break
            self.scan_token()
            self.start = self.current
        self.start = self.current
        
        

    def __is_alpha_numeric(self, c: str):
        return c.isalpha() or c.isnumeric()

    def __identifier(self):
        while self.__is_alpha_numeric(self.__peek()):
            self.__advance()
        
        text = self.source_code[self.start : self.current]
        token_type = self.reserved_keywords.get(text, None)
        if token_type is None:
            token_type = TokenType.IDENTIFIER
        self.__add_token(token_type)

    def __number(self):
        while self.__peek().isdigit():
            self.__advance()
        
        # check for fractional part.
        if self.__peek() == "." and self.__peek_next().isdigit():
            # Consume ".".
            self.__advance()

            while self.__peek().isdigit():
                self.__advance()
                
        self.__add_token(TokenType.NUMBER, float(self.source_code[self.start : self.current]))


    def __peek_next(self):
        if self.current + 1 >= len(self.source_code):
            return "\0"
        return self.source_code[self.current + 1]
    
    def __string(self):
        while self.__peek() != '"' and not self.__is_at_end():
            if self.__peek == "\n":
                self.line += 1
            self.__advance()

        if self.__is_at_end():
            error_reporter.error(self.line, "Unterminated string.")
            return

        # The closing ".
        self.__advance()
        
        # Trim the surrounding quotes.
        value = self.source_code[self.start + 1 : self.current - 1]
        self.__add_token(TokenType.STRING, value)


    def __peek(self) -> str:
        if self.__is_at_end():
            return "\0"
        return self.source_code[self.current]

    def __match_char(self, expected_char: str) -> bool:
        if self.__is_at_end():
            return False
        if self.source_code[self.current] != expected_char:
            return False
        
        self.current += 1
        return True
    

    def __advance(self) -> str:
        temp = self.current
        self.current += 1
        return self.source_code[temp]
    
    def __add_token(self, type: TokenType, literal: object = None):
        text = self.source_code[self.start : self.current]
        self.tokens.append(Token(type, text, literal, self.line))

    def __is_at_end(self):
        return self.current >= len(self.source_code)