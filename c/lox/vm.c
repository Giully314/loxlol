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


static InterpretResult run()
{
    #define READ_BYTE() (*vm.ip++)
    #define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

    while (true)
    {
        #ifdef DEBUG_TRACE_EXECUTION
        disassemble_instruction(vm.chunk, (uint32_t)(vm.ip - vm.chunk->code));
        #endif

        uint8_t instruction = READ_BYTE();
        switch (instruction)    
        {
        case OP_CONSTANT:
        {
            Value value = READ_CONSTANT();
            print_value(value);
            printf("\n");
            break;
        }

        case OP_RETURN:
        {
            return INTERPRET_OK;
        }
        
        default:
            break;
        }
    }


    #undef READ_BYTE
    #undef READ_CONSTANT
}



void init_vm()
{

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