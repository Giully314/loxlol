/*
lox/stack.c

TODO:
    after writing the compiler, compute the maximum size required for the stack and pre allocate it and remove
    the if statements.
*/

#include "stack.h"
#include "memory.h"
#include "value.h"
#include <stdlib.h>
#include <stdio.h>

void init_stack(Stack* stack)
{
    stack->s = NULL;
    stack->capacity = 0;
    stack->size = 0;
}


void push_stack(Stack* stack, Value value)
{
    if (stack->capacity < stack->size + 1)
    {
        uint32_t old_capacity = stack->capacity;
        stack->capacity = GROW_CAPACITY(old_capacity);
        stack->s = GROW_ARRAY(Value, stack->s, old_capacity, stack->capacity);
    }

    stack->s[stack->size++] = value;
}


Value pop_stack(Stack* stack)
{
    if (stack->size == 0)
    {
        // error!
        printf("[!] Stack underflow. Trying to pop an empty stack. Exit...");
        exit(1);
    }
    --stack->size;
    return stack->s[stack->size];
}


void free_stack(Stack* stack)
{
    FREE_ARRAY(Value, stack->s, stack->capacity);
    init_stack(stack);
}