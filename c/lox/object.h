
#ifndef OBJECT_H
#define OBJECT_H

#include "common.h"
#include "value.h"
#include "chunk.h"

#define OBJ_TYPE(value)     (AS_OBJ(value)->type)

#define IS_STRING(value)    is_obj_type(value, OBJ_STRING)
#define IS_FUNCTION(value)  is_obj_type(value, OBJ_FUNCTION)

#define AS_STRING(value)       ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)      (((ObjString*)AS_OBJ(value))->chars)
#define AS_FUNCTION(value)     ((ObjFunction*)AS_OBJ(value))

typedef enum
{
    OBJ_STRING,
    OBJ_FUNCTION,
} ObjType;


struct Obj
{
    ObjType type;
    Obj* next;
};


struct ObjString
{
    Obj obj;
    uint32_t size;
    uint32_t hash;  
    char chars[];
};

ObjString* concatenate_string(ObjString* s1, ObjString* s2);
ObjString* copy_string(const char* chars, uint32_t size);
ObjString* make_string(uint32_t size);


struct ObjFunction
{
    Obj obj;
    uint32_t arity;
    Chunk chunk;
    ObjString* name;
};

ObjFunction* new_function();



void print_object(Value value);


static inline bool is_obj_type(Value value, ObjType type) 
{
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif 