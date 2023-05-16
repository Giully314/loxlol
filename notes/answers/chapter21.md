# Exercise 1
The change is pretty simple. Add an hash table to the compiler and check inside identifier_constant if the name is already registered. In this way we use less memory in the value array at the small cost of a slower compilation (still fast since hash table is O(1)).

# Exercise 2
We can use a ValueArray only for global variables. We can use an hash table at compile time to save the idx of the name in the value array and then use it at run time inside the vm to get in O(1) the value. If the variable is not defined we can raise a compilation error or a run time error checking if the idx is a valid idx.


# Exercise 3
Even though Python is different (no prio declaration of the variable), it doesn't report the error until the execution of the function. I think we should report the error at runtime only if the function is executed if we want to allow the access of global variables before the definition.