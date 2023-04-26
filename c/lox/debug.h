/*
lox/debug.h


*/


#ifndef DEBUG_H
#define DEBUG_H

#include "chunk.h"
#include "common.h"

// Disassemble a chunk into its instructions and print them.
void disassemble_chunk(Chunk* chunk, const char* name);

// Decode a single instruction and print its name.
uint32_t disassemble_instruction(Chunk* chunk, uint32_t offset);

#endif