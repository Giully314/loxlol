import expression as expr
import loxtoken as t
import visitor

class ASTPrinter:
    # @visitor.visitor(expr.Expression)
    # def visit(self, e: expr.Expression):
    #     return "expression"
    
    @visitor.visitor(expr.Unary)
    def visit(self, e: expr.Unary):
        return self.__parenthesize(e.operator.lexeme, e.right)

    @visitor.visitor(expr.Binary)
    def visit(self, e: expr.Binary):
        return self.__parenthesize(e.operator.lexeme, e.left, e.right)
    

    @visitor.visitor(expr.Grouping)
    def visit(self, e: expr.Grouping):
        return self.__parenthesize("group", e.expr)
    
    @visitor.visitor(expr.Literal)
    def visit(self, e: expr.Literal):
        if e.value is None:
            return "nil"
        return str(e.value)
    

    @visitor.visitor(expr.Conditional)
    def visit(self, e: expr.Conditional):
        return f"if {self.visit(e.condition)} then {self.visit(e.then_branch)} else {self.visit(e.else_branch)}"
    

    def __parenthesize(self, name: str, *exprs: expr.Expression) -> str:
        s = [f"({name}"]
        for e in exprs:
            s.append(" ")
            s.append(self.visit(e))
        s.append(")")

        return "".join(s)
    

class ASTReversePolish:
    @visitor.visitor(expr.Unary)
    def visit(self, e: expr.Unary):
        ...

    @visitor.visitor(expr.Binary)
    def visit(self, e: expr.Binary):
        return f"{self.visit(e.left)} {self.visit(e.right)} {e.operator.lexeme}"

    # @visitor.visitor(expr.Grouping)
    # def visit(self, e: expr.Grouping):
    #     return self.__parenthesize("group", e.expr)
    
    @visitor.visitor(expr.Literal)
    def visit(self, e: expr.Literal):
        if e.value is None:
            return "nil"
        return str(e.value)


def test():
    e = expr.Binary(
        t.Token(t.TokenType.STAR, "*", None, 1),
        expr.Unary(t.Token(t.TokenType.MINUS, "-", None, 1), expr.Literal("123")),
        expr.Grouping(expr.Literal("45.1"))
    )

    e = expr.Binary(
        t.Token(t.TokenType.STAR, "*", None, 1),
        expr.Literal("12"),
        expr.Literal("1")
    )
    print(ASTReversePolish().visit(e))
    # print(ASTPrinter().visit(e))

if __name__ == "__main__":
    test()