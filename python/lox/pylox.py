import sys
from scanner import Scanner
from ast_printer import ASTPrinter
from loxparser import Parser
import error_reporter as err

class Lox:
    def __init__(self):
        self.ast_printer = ASTPrinter()

    def run(self, source_code):
        scanner = Scanner(source_code)
        tokens = scanner.scan_tokens()
        parser = Parser(tokens)
        expression = parser.parse()

        if err.error_reporter.had_error:
            return

        if expression is None:
            print("None expression")
            return

        print(ASTPrinter().visit(expression))
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