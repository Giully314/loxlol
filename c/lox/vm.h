/*
lox/vm.h

PURPOSE:

*/

#ifndef VM_H
#define VM_H    

#include "chunk.h"
#include "common.h"
#include "value.h"
#include "stack.h"

#define STACK_MAX 256


typedef struct
{
    Stack stack;

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
InterpretResult interpret(const char* source);
void free_vm();



#endif 