"""
A Resolver applies static analysis for resolving bindings (and more). After the program is parsed we need to check if the 
variables are binded to the right scope. For example, as reported in the book, the following code is "broken".

var a = "global";
{
  fun showA() {
    print a;
  }

  showA(); // print global
  var a = "block";
  showA(); // print block
}

To solve this problem we need to use a persistent environment that it is binded when the definition is encountered
the first time.
The solution to this problem is to compute the index of the list of scopes when the variable is encountered the first time
and pass the right environment to the interpreter. 


Another example is to check if a break statement or a return statement are actually valid. This type of things can be checked before 
execution.

"""

from enum import Enum, auto
from visitor import visitor
from dataclasses import dataclass, field
import statement as stmt
import expression as expr
from loxtoken import Token, TokenType
import error_reporter as err
from loxinterpreter import Interpreter

class FunctionType(Enum):
    NONE = auto()
    FUNCTION = auto()



@dataclass
class Resolver:
    interpreter: Interpreter
    scopes: list[dict[str, bool]] = field(default_factory=list, init=False)
    current_function: FunctionType = field(default=FunctionType.NONE, init=False)
    inside_loop: int = field(default=0, init=False)

    def resolve(self, statements: list[stmt.Statement]):
        print("START Resolve")
        for statement in statements:
            self.visit(statement)
        print("END Resolve")


    @visitor(stmt.Block)
    def visit(self, s: stmt.Block):
        self.__begin_scope()
        self.resolve(s.statements)
        self.__end_scope()


    @visitor(stmt.Var)
    def visit(self, s: stmt.Var):
        self.__declare(s.name)
        if s.initializer is not None:
            self.visit(s.initializer)
        self.__define(s.name)


    @visitor(stmt.StmtExpression)
    def visit(self, s: stmt.StmtExpression):
        self.visit(s.expr)    


    @visitor(stmt.If)
    def visit(self, s: stmt.If):
        self.visit(s.condition)
        self.visit(s.then_branch)
        if s.else_branch is not None:
            self.visit(s.else_branch)
        

    @visitor(stmt.Print)
    def visit(self, s: stmt.Print):
        self.visit(s.expr)
    
    
    @visitor(stmt.Return)
    def visit(self, s: stmt.Return):
        if self.current_function == FunctionType.NONE:
            err.error_reporter.error_token(s.keyword, "Can't return from top-level code.")

        if s.value is not None:
            self.visit(s.value)


    @visitor(stmt.While)
    def visit(self, s: stmt.While):
        self.inside_loop += 1
        self.visit(s.condition)
        self.visit(s.body)
        self.inside_loop -= 1


    @visitor(stmt.Break)
    def visit(self, s: stmt.Break):
        if self.inside_loop == 0:
            err.error_reporter.error_token(s.keyword, "Can't use break outside a loop.")
            

    @visitor(stmt.Function)
    def visit(self, s: stmt.Function):
        self.__declare(s.name)
        self.__define(s.name)

        self.__resolve_function(s, FunctionType.FUNCTION)

    
    @visitor(expr.Variable)
    def visit(self, e: expr.Variable):
        if self.scopes and self.scopes[-1].get(e.name.lexeme) == False:
            err.error_reporter.error_token(e.name, "Can't read local variable in its own initializer.")

        self.__resolve_local(e, e.name)


    @visitor(expr.Assign)
    def visit(self, e: expr.Assign):
        self.visit(e.value)
        self.__resolve_local(e, e.name)


    @visitor(expr.Binary)
    def visit(self, e: expr.Binary):
        self.visit(e.left)
        self.visit(e.right)
    

    @visitor(expr.Call)
    def visit(self, e: expr.Call):
        self.visit(e.callee)
        for arg in e.arguments:
            self.visit(arg)


    @visitor(expr.Grouping)
    def visit(self, e: expr.Grouping):
        self.visit(e.expr)
    
    @visitor(expr.Literal)
    def visit(self, e: expr.Literal):
        ...
    

    @visitor(expr.Logical)
    def visit(self, e: expr.Logical):
        self.visit(e.left)
        self.visit(e.right)


    @visitor(expr.Unary)
    def visit(self, e: expr.Unary):
        self.visit(e.right)


    def __resolve_function(self, s: stmt.Function, func_type: FunctionType):
        enclosing = self.current_function
        self.current_function = func_type

        self.__begin_scope()

        for token in s.params:
            self.__declare(token)
            self.__define(token)
        
        self.resolve(s.body)

        self.__end_scope()

        self.current_function = enclosing

    def __resolve_local(self, e: expr.Expression, name: Token):
        for i in range(len(self.scopes) - 1, -1, -1):
            if name.lexeme in self.scopes[i]:
                print(f"idx:{i} distance: {len(self.scopes) - 1 - i} {name.lexeme}\n{self.scopes[i]}")
                self.interpreter._resolve(e, len(self.scopes) - 1 - i)
                return

    def __declare(self, name: Token):
        if not self.scopes:
            return
        
        if name.lexeme in self.scopes[-1]:
            err.error_reporter.error_token(name, "Already a variable with this name in this scope.")

        self.scopes[-1][name.lexeme] = False


    def __define(self, name: Token):
        if not self.scopes:
            return
        self.scopes[-1][name.lexeme] = True

    def __begin_scope(self):
        print("begin scope")
        self.scopes.append(dict())

    def __end_scope(self):
        print("end scope")
        self.scopes.pop()
    