/*
lox/vm.c
*/

#include "vm.h"
#include "chunk.h"
#include "value.h"
#include "common.h"
#include "debug.h"

#include <stdio.h>

// Global variable. It's ok to use this approach for simplicity because there is only one vm.
VM vm;


static void print_stack()
{
    printf("          ");
    for (Value* e = vm.stack; e < vm.stack_top; ++e)
    {
        printf("[ ");
        print_value(*e);
        printf(" ]");
    }
    printf("\n");
}


static InterpretResult run()
{
    #define READ_BYTE() (*vm.ip++)
    #define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
    #define READ_CONSTANT_LONG() (vm.chunk->constants.values[\
        ((0u | READ_BYTE()) << 8\
             | READ_BYTE())  << 8\
             | READ_BYTE()])

    #define BINARY_OP(op) \
        do { \
            double b = pop_stack(); \
            double a = pop_stack(); \
            push_stack(a op b); \
        } while (false)



    while (true)
    {
        #ifdef DEBUG_TRACE_EXECUTION
        print_stack();
        disassemble_instruction(vm.chunk, (uint32_t)(vm.ip - vm.chunk->code));
        #endif

        uint8_t instruction = READ_BYTE();
        switch (instruction)    
        {
        case OP_ADD: 
        {
            BINARY_OP(+);
            break;
        }
        case OP_SUBTRACT: 
        {
            BINARY_OP(-);
            break;
        }
        case OP_MULTIPLY: 
        {
            BINARY_OP(*);
            break;
        }
        case OP_DIVIDE: 
        {
            BINARY_OP(/);
            break;
        }

        case OP_NEGATE: 
        {
            push_stack(-pop_stack());
            break;
        }
        case OP_CONSTANT_LONG:
        {
            Value value = READ_CONSTANT_LONG();
            push_stack(value);
            break;
        }
        case OP_CONSTANT:
        {
            Value value = READ_CONSTANT();
            push_stack(value);
            break;
        }

        case OP_RETURN:
        {
            print_value(pop_stack());
            printf("\n");
            return INTERPRET_OK;
        }
        
        default:
            break;
        }
    }


    #undef READ_BYTE
    #undef READ_CONSTANT
    #undef READ_CONSTANT_LONG
    #undef BINARY_OP
}


static void reset_stack()
{
    vm.stack_top = vm.stack;
}


void init_vm()
{
    reset_stack();
}


InterpretResult interpret(Chunk* chunk)
{
    vm.chunk = chunk;
    vm.ip = chunk->code;
    return run();
}

void free_vm()
{

}


void push_stack(Value value)
{
    *vm.stack_top++ = value; 
}


Value pop_stack()
{
    --vm.stack_top;
    return *vm.stack_top;
}