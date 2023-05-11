
#ifndef OBJECT_H
#define OBJECT_H

#include "common.h"
#include "value.h"

#define OBJ_TYPE(value)     (AS_OBJ(value)->type)

#define IS_STRING(value)    is_obj_type(value, OBJ_STRING)

#define AS_STRING(value)       ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)      (((ObjString*)AS_OBJ(value))->chars)

typedef enum
{
    OBJ_STRING,
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
    char chars[];
};


ObjString* copy_string(const char* chars, uint32_t size);
ObjString* make_string(uint32_t size);

void print_object(Value value);


static inline bool is_obj_type(Value value, ObjType type) 
{
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif 