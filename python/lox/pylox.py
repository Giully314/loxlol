import sys
from scanner import Scanner
from ast_printer import ASTPrinter
from loxparser import Parser
from loxinterpreter import Interpreter
import error_reporter as err

class Lox:
    def __init__(self):
        self.ast_printer = ASTPrinter()
        self.interpreter = Interpreter()

    def run(self, source_code):
        scanner = Scanner(source_code)
        tokens = scanner.scan_tokens()
        parser = Parser(tokens)
        expression = parser.parse() 

        # TODO: make a better error checking.
        if err.error_reporter.had_error or err.error_reporter.had_runtime_error:
            return

        if expression is None:
            print("None expression")
            return
        
        self.interpreter.interpret(expression)

        # print(ASTPrinter().visit(expression))
        # for token in tokens:
        #     # print(token)
        #     print(f"{token.type.name}")
        #     # self.ast_printer.visit(token)


    def run_file(self):
        pass

    def run_prompt(self):
        while True:
            s = input("Lox>>: ")

            if s == "exit()":
                break
            self.run(s)

def main():
    args = sys.argv

    lox = Lox()

    lox.run_prompt()


if __name__ == "__main__":
    main()