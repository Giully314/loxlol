/*
lox/value.c
*/

#include "value.h"
#include "memory.h"
#include "common.h"
#include <stdio.h>


void init_value_array(ValueArray* array)
{
    array->values = NULL;
    array->size = 0;
    array->capacity = 0;
}


void write_value_array(ValueArray* array, Value value)
{
    if (array->capacity < array->size + 1)
    {
        uint32_t old_capacity = array->capacity;
        array->capacity = GROW_CAPACITY(old_capacity);
        array->values = GROW_ARRAY(Value, array->values, old_capacity, array->capacity);
    }
    
    array->values[array->size++] = value;
}


void free_value_array(ValueArray* array)
{
    FREE_ARRAY(Value, array->values, array->capacity);
    init_value_array(array);
}


void print_value(Value value)
{
    printf("%g", value);
}