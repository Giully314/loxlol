/*
lox/value.h

PURPOSE:
    Represents a value in the lox VM.

STRUCT:
    ValueArray:

*/


#ifndef VALUE_H
#define VALUE_H

#include "common.h"


typedef double Value;


typedef struct
{
    Value* values;

    // Number of elements.
    uint32_t size;

    // Capacity of the array before allocating again.
    uint32_t capacity;
} ValueArray;

// Init ValueArray.
void init_value_array(ValueArray* array);

// Add a value to the array. Reallocate the array if necessary.
void write_value_array(ValueArray* array, Value value);

// Free the memory.
void free_value_array(ValueArray* array);

void print_value(Value value);


#endif 