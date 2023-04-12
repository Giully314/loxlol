# Exercise 1
Conditional execution can be build using polymorphism (dynamic dispatch). The basic idea is to create two class True and False that call the method passed based on the condition. I don't know any language that uses this method to implement conditionals. 

For example in C++:  

    // We can use singleton or static functions.
    
    class True 
    {
        public:
            template <typename F>
                requires invocable<F> 
            static void if_then(F&& then_branch)
            {
                then_branch();
            }

            template <typename F, typename W>
                requires invocable<F> and invocable<W> 
            static void if_then_else(F&& then_branch, W&& else_branch)
            {
                then_branch();
            }
    };

    class False
    {
        public:
            template <typename F>
                requires invocable<F> 
            static void if_then(F&& then_branch)
            {
                return;
            }

            template <typename F, typename W>
                requires invocable<F> and invocable<W> 
            static void if_then_else(F&& then_branch, W&& else_branch)
            {
                else_branch();
            }
    };



# Exercise 2
To simulate loops we can use recursion. One main problem of this solution is that we can do possibly thousands of recursive calls, using all the stack. We can avoid this problem implementing tail optimization when possible. I think functional languages use this optimization because recursion and function calls are at the base of the language but i don't know any name.