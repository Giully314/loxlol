/*
lox/vm.c
*/

#include "vm.h"
#include "chunk.h"
#include "value.h"
#include "common.h"
#include "debug.h"
#include "stack.h"

#include <stdio.h>

// Global variable. It's ok to use this approach for simplicity because there is only one vm.
VM vm;


static void print_stack()
{
    printf("          ");
    for (uint32_t i = 0; i < vm.stack.size; ++i)
    {
        printf("[ ");
        print_value(vm.stack.s[i]);
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
            double b = pop_stack(&vm.stack); \
            vm.stack.s[vm.stack.size-1] op##= b;\
        } while (false)

            // double a = pop_stack(&vm.stack); 
            //push_stack(&vm.stack, a op b); 


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
            uint32_t top = vm.stack.size - 1;
            vm.stack.s[top] = -vm.stack.s[top];
            break;
        }
        case OP_CONSTANT_LONG:
        {
            Value value = READ_CONSTANT_LONG();
            push_stack(&vm.stack, value);
            break;
        }
        case OP_CONSTANT:
        {
            Value value = READ_CONSTANT();
            push_stack(&vm.stack, value);
            break;
        }

        case OP_RETURN:
        {
            print_value(pop_stack(&vm.stack));
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
    vm.stack.size = 0;
}


void init_vm()
{
    init_stack(&vm.stack);
}


InterpretResult interpret(Chunk* chunk)
{
    vm.chunk = chunk;
    vm.ip = chunk->code;
    return run();
}

void free_vm()
{
    free_stack(&vm.stack);
}
