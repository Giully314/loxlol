# Exercise 1
1.
+  OP_CONSTANT 1 
+  OP_CONSTANT 2 
+  OP_MULTIPLY
+  OP_CONSTANT 3
+  OP_ADD 

3.
+ OP_CONSTANT 3
+ OP_CONSTANT 2
+ OP_NEGATE
+ OP_ADD 
+ OP_CONSTANT 1
+ OP_NEGATE
+ OP_ADD 

4.
+ OP_CONSTANT 1
+ OP_CONSTANT 2
+ OP_CONSTANT 3
+ OP_MULTIPLY
+ OP_CONSTANT 4
+ OP_NEGATE
+ OP_CONSTANT 5
+ OP_NEGATE
+ OP_DIVIDE
+ OP_ADD
+ OP_ADD


# Exercise 2
Without OP_NEGATE  
+ OP_CONSTANT 4
+ OP_CONSTANT 3
+ OP_CONSTANT 0
+ OP_CONSTANT 2
+ OP_SUBTRACT
+ OP_MULTIPLY
+ OP_SUBTRACT  

Without OP_SUBTRACT  
+ OP_CONSTANT 4
+ OP_CONSTANT 3
+ OP_NEGATE
+ OP_CONSTANT 2
+ OP_NEGATE
+ OP_MULTIPLY
+ OP_ADD  

Using both  
+ OP_CONSTANT 4
+ OP_CONSTANT 3
+ OP_CONSTANT 2
+ OP_NEGATE
+ OP_MULTIPLY
+ OP_SUBTRACT 

It makes sense to have both instructions because it simplifies some expressions and could perform less instructions as we can see in the above case. 


# Exercise 3
The benefits are safety and dynamic growth without worrying too much about the space. The cost are more operations in push and pop (for example checking the capacity in push or checking if the stack is empty in pop).
The problem could be solved at compile time. We can know the maximum size we need for the stack and set it at the start of the vm.  


# Exercise 4
We can do a similar approach for the binary operations. There's no need to double pop and then push. We can just 
pop and inplace perform the operation on top of the stack.