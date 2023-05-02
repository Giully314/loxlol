/*
lox/chunk.c
*/

#include "chunk.h"
#include "value.h"
#include "common.h"
#include "memory.h"


// ******************************* LINE ARRAY *********************************************


void init_line_array(LineArray* array)
{
    array->lines = NULL;
    array->size = 0;
    array->capacity = 0;
}

void write_new_line(LineArray* array, uint32_t offset, uint32_t line)
{
    // Same line
    if (array->size > 0 && array->lines[array->size-1].line == line)
    {
        return;
    }

    if (array->capacity < array->size + 1)
    {
        uint32_t old_capacity = array->capacity;
        array->capacity = GROW_CAPACITY(old_capacity);
        array->lines = GROW_ARRAY(LinePair, array->lines, old_capacity, array->capacity);
    }

    array->lines[array->size].offset = offset;
    array->lines[array->size].line = line;
    ++array->size;
}

// TODO: instead of linear search we could use binary search.
uint32_t get_line(LineArray* array, uint32_t offset)
{
    for (uint32_t i = 0; i < array->size; ++i)
    {
        if (offset < array->lines[i].offset)
        {
            return array->lines[i-1].line;
        }
    }

    return array->lines[array->size-1].line;
}


// uint32_t get_if_not_same_line(LineArray* array, uint32_t value1, uint32_t value2)
// {
//     for (uint32_t i = 0; i < array->size; ++i)
//     {    
//         if (value1 < array->lines[i] || value2 < array->lines[i])
//         {
//             // We need to do an additional check to see if they are in the same line.
//             if (value1 < array->lines[i] && value2 < array->lines[i])
//             {
//                 return 0;
//             }
//             else // return the line of the value2
//             {
//                 for (uint32_t j = i+1; j < array->size; ++j)
//                 {
//                     if (value2 < array->lines[j])
//                     {
//                         return j + 1;
//                     } 
//                 }
//             }
//         }
//     }
//     // This should never happen.
//     return 0;
// }


void free_line_array(LineArray* array)
{
    FREE_ARRAY(LinePair, array->lines, array->capacity);
    init_line_array(array);
}


// ******************************* CHUNK *********************************************


void init_chunk(Chunk* chunk)
{
    chunk->code = NULL;
    chunk->size = 0;
    chunk->capacity = 0;
    init_value_array(&chunk->constants);
    init_line_array(&chunk->lines);
}


void write_chunk(Chunk* chunk, uint8_t byte, uint32_t line)
{
    if (chunk->capacity < chunk->size + 1)
    {
        uint32_t old_capacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(old_capacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, old_capacity, chunk->capacity);
    }

    // This update must be done before the line update because we need to use size as upper bound (not included).
    chunk->code[chunk->size++] = byte;

    write_new_line(&chunk->lines, chunk->size-1, line);
}


uint32_t add_constant(Chunk* chunk, Value value)
{
    write_value_array(&chunk->constants, value);
    return chunk->constants.size - 1;   
}


void write_constant(Chunk* chunk, Value value, uint32_t line)
{
    uint32_t idx = add_constant(chunk, value);

    if (idx < MAX_NUM_OF_CONSTS)
    {
        write_chunk(chunk, OP_CONSTANT, line);
        write_chunk(chunk, (uint8_t)idx, line);
    }
    else
    {
        write_chunk(chunk, OP_CONSTANT_LONG, line);
        
        // Write the 3 bytes in big endian mode.
        uint8_t c1 = idx & 0xFF;
        idx = idx >> 8;
        uint8_t c2 = idx & 0xFF;
        idx = idx >> 8;
        uint8_t c3 = idx & 0xFF;
        idx = idx >> 8;
        
        write_chunk(chunk, c3, line);
        write_chunk(chunk, c2, line);
        write_chunk(chunk, c1, line);
    }
}


void free_chunk(Chunk* chunk)
{
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    free_line_array(&chunk->lines);
    free_value_array(&chunk->constants);
    init_chunk(chunk);
}

