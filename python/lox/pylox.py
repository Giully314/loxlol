import sys
from scanner import Scanner
# from ast_printer import ASTPrinter
from loxparser import Parser
from loxinterpreter import Interpreter
import error_reporter as err
import statement as stmt

class Lox:
    def __init__(self):
        self.interpreter = Interpreter()

    def run(self, source_code):
        scanner = Scanner(source_code)
        tokens = scanner.scan_tokens()
        parser = Parser(tokens)
        statements = parser.parse() 

        # TODO: make a better error checking.
        if err.error_reporter.had_error or err.error_reporter.had_runtime_error:
            return

        # if expression is None:
        #     print("None expression")
        #     return
        
        self.interpreter.interpret(statements)


    def run_file(self, filename: str):
        with open(filename) as f:
            source_code = f.read()
        self.run(source_code)

    def run_prompt(self):
        while True:
            s = input("Lox>>: ")

            if s == "exit()":
                break
            
            scanner = Scanner(s)
            tokens = scanner.scan_tokens()
            parser = Parser(tokens)
            statements = parser.parse()
            
            modified_statements = []
            for statement in statements:
                if isinstance(statement, stmt.StmtExpression):
                    modified_statements.append(stmt.Print(statement.expr))
                else:
                    modified_statements.append(statement)

            self.interpreter.interpret(modified_statements)
        

def main():
    args = sys.argv

    lox = Lox()

    if len(args) == 1:
        lox.run_prompt()
    else:
        lox.run_file(args[1])


if __name__ == "__main__":
    main()