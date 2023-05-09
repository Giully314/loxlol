# Exercise 1
We can remove OP_FALSE (or OP_TRUE) and use OP_FALSE OP_NOT to represent true. Same thing with OP_GREATER, we can use OP_LESS OP_NOT. 


# Exercise 2
We can add >=, =<, != as native instructions. Also add/subtract 1 could be a common instruction. I don't know if it's worth to add instructions for 0 and 1 like we did for true and false.