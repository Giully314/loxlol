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

typedef enum 
{
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
} ValueType;


typedef struct
{
    ValueType type;
    union
    {
        bool boolean;
        double number;
    } as;
} Value;


#define BOOL_VAL(value)     ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL             ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value)   ((Value){VAL_NUMBER, {.number = value}})

#define AS_BOOL(value)      ((value).as.boolean)
#define AS_NUMBER(value)    ((value).as.number)


#define IS_BOOL(value)      ((value).type == VAL_BOOL)
#define IS_NIL(value)       ((value).type == VAL_NIL)
#define IS_NUMBER(value)    ((value).type == VAL_NUMBER)



typedef struct
{
    Value* values;

    // Number of elements.
    uint32_t size;

    // Capacity of the array before allocating again.
    uint32_t capacity;
} ValueArray;


bool values_equal(Value a, Value b);

// Init ValueArray.
void init_value_array(ValueArray* array);

// Add a value to the array. Reallocate the array if necessary.
void write_value_array(ValueArray* array, Value value);

// Free the memory.
void free_value_array(ValueArray* array);

void print_value(Value value);


#endif 