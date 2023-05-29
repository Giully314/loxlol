/*
lox/vm.h

PURPOSE:

*/

#ifndef VM_H
#define VM_H    

#include "chunk.h"
#include "common.h"
#include "value.h"
#include "object.h"
#include "stack.h"
#include "object.h"
#include "table.h"

#define FRAMES_MAX 64
// #define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct
{
    ObjFunction* function;
    uint8_t* ip;
    Value* slots;
} CallFrame;


typedef struct
{
    CallFrame frames[FRAMES_MAX];
    uint32_t frame_count;

    Stack stack;
    HashTable strings;
    HashTable globals;

    register uint8_t* ip;

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