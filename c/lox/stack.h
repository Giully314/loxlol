/*
lox/stack.h

PURPOSE:
    Dynamic stack.

STRUCT:

*/

#ifndef STACK_H
#define STACK_H

#include "value.h"
#include "common.h"

typedef struct
{
    Value* s;
    uint32_t size;
    uint32_t capacity;
} Stack;

void init_stack(Stack* stack);
void push_stack(Stack* stack, Value value);
Value pop_stack(Stack* stack);
void free_stack(Stack* stack);


#endif