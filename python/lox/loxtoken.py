from dataclasses import dataclass
from enum import Enum

TokenType = Enum("TokenType", [
    # Single-character tokens.
    "LEFT_PAREN", "RIGHT_PAREN", "LEFT_BRACE", "RIGHT_BRACE",
    "COMMA", "DOT", "MINUS", "PLUS", "SEMICOLON", "SLASH", "STAR", "QUESTION", "COLON",

  # One or two character tokens. Note: BANG stands for the sign "!".
    "BANG", "BANG_EQUAL",
    "EQUAL", "EQUAL_EQUAL", 
    "GREATER", "GREATER_EQUAL",
    "LESS", "LESS_EQUAL",

  # Literals.
    "IDENTIFIER", "STRING", "NUMBER",

  # Keywords.
    "AND", "CLASS", "ELSE", "FALSE", "FUN", "FOR", "IF", "NIL", "OR",
    "PRINT", "RETURN", "SUPER", "THIS", "TRUE", "VAR", "WHILE", "BREAK",

    "EOF"
])


@dataclass(frozen=True)
class Token:
    type: TokenType
    lexeme: str
    literal: object
    line: int

    def __str__(self) -> str:
        return f"{self.type} {self.lexeme} {self.literal}"