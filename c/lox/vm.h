/*
lox/vm.h

PURPOSE:

*/

#ifndef VM_H
#define VM_H    

#include "chunk.h"
#include "common.h"
#include "value.h"

#define STACK_MAX 256


typedef struct
{
    Value stack[STACK_MAX];
    // Point to the first free space.
    Value* stack_top;

    Chunk* chunk;

    // Instruction pointer.
    uint8_t* ip;
} VM;


typedef enum 
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;


void init_vm();
InterpretResult interpret(Chunk* chunk);
void free_vm();

void push_stack(Value value);
Value pop_stack();


#endif 