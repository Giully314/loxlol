# This file is a metaprogramming script that generates the code for all types of expressions.  

import sys
import os
import io


def define_ast(output_dir: str, output_file: str, baseclass_name: str, subclasses: list[str]):
    """ 
    subclasses: list of types that inherit from baseclass; 
    "type_name $ attr_name1: type1, attr_name2: type2, ..."
    """
    output_file = os.path.join(output_dir, output_file) 
    with open(output_file, "w") as f:
        # import modules and generate base class with the abstract method accept(visitor).
        f.write("from loxtoken import Token\n")
        f.write("from abc import ABC\n")
        f.write("from dataclasses import dataclass\n\n")
        f.write(f"class {baseclass_name}(ABC):\n    ...\n\n")
        # f.write(f"    @abstractmethod\n")
        # f.write(f"    def accept(visitor):\n        ...\n\n\n")

        for subclass in subclasses:
            s = subclass.split("$")
            subclass_name = s[0].strip()
            fields = s[1].split(",")
            define_types(f, baseclass_name, subclass_name, fields)
            f.write("\n\n")

def define_types(file: io.TextIOWrapper, baseclass: str, class_name: str, fields: list[str]):
    file.write(f"@dataclass(frozen=True)\nclass {class_name}({baseclass}):\n")
    for field in fields:
        file.write(f"   {field}\n")

def define_visitors():
    pass


def main():
    args = sys.argv 

    if len(args) == 1:
        print("Usage: python expression_generator.py output_dir expr_output_file stmt_output_file")
        return

    subclasses = [
        "Unary       $ operator: Token, right: Expression",
        "Grouping    $ expr: Expression",
        "Literal     $ value: object",
        "Binary      $ operator: Token, left: Expression, right: Expression",
        "Variable    $ name: Token",
        "Assign      $ name: Token, value: Expression",
        "Conditional $ condition: Expression, then_branch: Expression, else_branch: Expression",
        "Logical     $ operator: Token, left: Expression, right: Expression",
    ]

    stmt_subclasses = [
        "StmtExpression     $ expr: Expression",
        "Print              $ expr: Expression",
        "Var                $ name: Token, initializer: Expression",
        "Block              $ statements: list[Statement]",
        "If                 $ condition: Expression, then_branch: Expression, else_branch: Expression"
    ]

    define_ast(args[1], args[2], "Expression", subclasses)
    define_ast(args[1], args[3], "Statement", stmt_subclasses)


if __name__ == "__main__":
    main() 
