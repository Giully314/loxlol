# Exercise 1
I don't know the Smalltalk language but after some reading on this [page](https://en.wikipedia.org/wiki/Smalltalk), i think it's for the fact that the language is first compiled into byte code and then executed. In the compilation phase we have access to the declaration and the calls and so we can check these types of errors. (This is a wrong answer).  
After checking the right answer and reading more about Smalltalk i understand that the way function calls (messages in the language) work is based on composition of method names followed by arguments. This means that we can't pass more or less arguments to a method because it wouldn't refer to the same type of message. From my understanding we can pass only one argument to a message so the check of arguments is implicit while parsing.

# Exercise 2
We can use the bind a name to an anonymous function to reuse the code already implemented. This name is internal
to the interpreter.