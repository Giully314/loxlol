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
#include "object.h"
#include "table.h"

// #define STACK_MAX 256


typedef struct
{
    Stack stack;
    HashTable strings;
    HashTable globals;

    Chunk* chunk;

    // Instruction pointer.
    uint8_t* ip;
    Obj* objects;
} VM;


typedef enum 
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;


extern VM vm;


void init_vm();
InterpretResult interpret(const char* source);
void free_vm();



#endif 