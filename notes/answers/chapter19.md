# Exercise 1
The changes are simple. Check the code for the full implementation.  

    struct ObjString
    {
        uint32_t size;
        char s[];
    };

    // For allocation
    malloc(sizeof(struct ObjString) + size);

Like stated in the excercise, the main advantage is to have all the data in a single block of memory.



# Exercise 3
I would report an error. I don't like silent conversions between types.