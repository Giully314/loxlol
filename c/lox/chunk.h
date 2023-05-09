/*
lox/chunk.h

PURPOSE:

STRUCT:
    LineArray: Dynamic array for info about the line of a bytecode.
    Chunk: Dynamic array for opcodes.

DESCRIPTION:

*/

#ifndef CHUNK_H
#define CHUNK_H

#include "common.h"
#include "value.h"


// after this number, OP_CONST_LONG will be used.
// 256 = 2^8 
#define MAX_NUM_OF_CONSTS 256


typedef enum 
{
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_CONSTANT_LONG, // support 24 bit operand for constant
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_RETURN,
} OpCode;

typedef struct
{
    uint32_t offset;
    uint32_t line;
} LinePair;
typedef struct
{    
    LinePair *lines;
    uint32_t size;
    uint32_t capacity;
} LineArray;


void init_line_array(LineArray* array);


// Write a new line and in case reallocate the array for new memory.
void write_new_line(LineArray* array, uint32_t offset, uint32_t line);

// Precondition: offset is a valid number.
// offset is the idx of the bytecode whose line we want.
uint32_t get_line(LineArray* array, uint32_t offset);

// Precondition value1 <= value2
// Return 0 if value1 and value2 are in the same line, else the return the line of value2.
uint32_t get_if_not_same_line(LineArray* array, uint32_t value1, uint32_t value2);

void free_line_array(LineArray* array);


typedef struct
{
    ValueArray constants;   
    LineArray lines;

    uint8_t* code;


    // Number of elements.
    uint32_t size; 
    
    // Capacity of the array before allocating again.
    uint32_t capacity;
} Chunk;

// Init an empty chunk.
void init_chunk(Chunk* chunk);

// Add an opcode to the chunk. Reallocate the array if necessary.
void write_chunk(Chunk* chunk, uint8_t byte, uint32_t line);

// Exercise 14.2. Add support for constants with 24 bit operand.
void write_constant(Chunk* chunk, Value value, uint32_t line);

uint32_t add_constant(Chunk* chunk, Value value);

// Free the memory.
void free_chunk(Chunk* chunk);

#endif

