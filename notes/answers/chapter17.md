# Exercise 1
expression  
-parse(assignment)  
--grouping  
---expression  
----parse(assignment)  
-----unary // for minus  
------parse(unary)  
-------number  
-----binary // for +  
------parse(factor)  
-------number  
--binary // for *  
---parse(unary)  
----number  
--binary // for -  
---parse(factor)  
----unary  
-----parse(unary)  
------number  

# Exercise 2
In lox there is the '(' as prefix operator.
In the C language * (dereference) and & (address of) are prefix operators.

