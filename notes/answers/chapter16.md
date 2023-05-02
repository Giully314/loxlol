# Exercise 1
I would introduce a TOKEN_FORMATTED_STRING to handle this type
of strings. The token could contain a list of tokens relative to the formatted expressions. One change that i would introduce is to have a special symbol before the string like in python (f"") or c# (\$"") to simplify the recognition in the scanner. Another method is just to have a function which formats the string like in C++ the function format("{}", 3 +4). In this case we don't need to distinguish between the 2 token types.

# Exercise 2
The first thing that comes in my mind to solve this problem is to just have some sort of variable that tells me if we found or not '<' characters so we can use it to match '>' and check if are in generic expression or not.


# Exercise 3
I don't like contextual keywords because they could introduce confusion. I don't know any contextual keywords (my language's pool is very limited, C, C++ and Python).
I would check in the parser if the current token that uses a contextual keyword is in a reserved context or not. 
