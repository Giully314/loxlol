from dataclasses import dataclass, field 
from loxtoken import Token, TokenType

@dataclass(init=False)
class ErrorReporter:
    had_error: bool = False
    
    def report(self, line: int, where: str, msg: str):
        print(f"[line {line}] Error {where}: {msg}")

    def error(self, line: int, msg: str):
        self.report(line, "", msg)
        self.had_error = True

    def error_token(self, token: Token, msg: str):
        if token.type == TokenType.EOF:
            self.report(token.line, " at end", msg)
        else:
            self.report(token.line, " at '" + token.lexeme + "'", msg)
        self.had_error = True



# Note: global objects usually are bad (specially if mutable and accessed by different parts of the program)
# but in this case the goal is to learn how to make an interpreter so i will gliss a little bit on the 
# basic principles of software architecture.
error_reporter = ErrorReporter()
