/*
lox/debub.c
*/
#include "debug.h"
#include "chunk.h"
#include "common.h"
#include "value.h"

#include <stdio.h>


void disassemble_chunk(Chunk* chunk, const char* name)
{
    printf("== %s ==\n", name);

    for (uint32_t offset = 0; offset < chunk->size;)
    {
        offset = disassemble_instruction(chunk, offset);
    }
}


static uint32_t simple_instruction(const char* name, uint32_t offset)
{
    printf("%s\n", name);
    return offset + 1;
}

static uint32_t constant_long_instruction(const char* name, Chunk* chunk, uint32_t offset)
{
    // Extract the 3 bytes.
    uint32_t constant = 0;
    constant = constant | chunk->code[offset + 1]; 
    constant = constant << 8;
    constant = constant | chunk->code[offset + 2]; 
    constant = constant << 8;
    constant = constant | chunk->code[offset + 3]; 

    printf("%-16s %4u '", name, constant);
    print_value(chunk->constants.values[constant]);
    printf("'\n");
    return offset + 4;
}

static uint32_t constant_instruction(const char* name, Chunk* chunk, uint32_t offset)
{
    uint8_t constant = chunk->code[offset + 1];
    printf("%-16s %4u '", name, constant);
    print_value(chunk->constants.values[constant]);
    printf("'\n");
    return offset + 2;
}


uint32_t disassemble_instruction(Chunk* chunk, uint32_t offset)
{
    printf("%04u ", offset);

    if (offset > 0) // if offset is 0 we can't pass offset-1 (underflow because of uint)
    {
        uint32_t line = get_if_not_same_line(&chunk->lines, offset, offset - 1);
        if (offset > 0 && line == 0)
        {
            printf("   | ");
        }
        else
        {
            printf("%4u ", line);
        }
    }
    else
    {
        printf("%4u ", get_line(&chunk->lines, offset));
    }

    uint8_t instruction = chunk->code[offset];
    switch (instruction)
    {
        case OP_ADD:
            return simple_instruction("OP_ADD", offset);
        case OP_SUBTRACT:
            return simple_instruction("OP_SUBTRACT", offset);
        case OP_MULTIPLY:
            return simple_instruction("OP_MULTIPLY", offset);
        case OP_DIVIDE:
            return simple_instruction("OP_DIVIDE", offset);
            
        case OP_NEGATE:
            return simple_instruction("OP_NEGATE", offset);
        
        case OP_CONSTANT_LONG:
            return constant_long_instruction("OP_CONSTANT_LONG", chunk, offset);
        case OP_CONSTANT:
            return constant_instruction("OP_CONSTANT", chunk, offset);
        
        case OP_RETURN:
            return simple_instruction("OP_RETURN", offset);
        default:
            printf("Unknown opcode %u\n", instruction);
            return offset + 1;
    }
}

