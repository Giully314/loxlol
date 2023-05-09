/*
lox/value.c
*/

#include "value.h"
#include "memory.h"
#include "common.h"
#include <stdio.h>


bool values_equal(Value a, Value b)
{
    if (a.type != b.type) return false;
    switch (a.type) 
    {
        case VAL_BOOL:   return AS_BOOL(a) == AS_BOOL(b);
        case VAL_NIL:    return true;
        case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
        default:         return false; // Unreachable.
    }
}

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
    switch (value.type) 
    {
        case VAL_BOOL:
            printf(AS_BOOL(value) ? "true" : "false");
            break;
        case VAL_NIL: printf("nil"); break;
        case VAL_NUMBER: printf("%g", AS_NUMBER(value)); break;
    }
}