
#include "object.h"
#include "memory.h"
#include "value.h"
#include "common.h"
#include "vm.h"

#include <stdio.h>
#include <string.h>

#define ALLOCATE_OBJ(type, object_type) \
    (type*)allocate_object(sizeof(type), object_type)


static Obj* allocate_object(uint32_t size, ObjType type)
{
    Obj* obj = reallocate(NULL, 0, size);
    obj->type = type;

    obj->next = vm.objects;
    vm.objects = obj;
    return obj;
}


// static ObjString* allocate_string(char* chars, uint32_t size)
// {
//     ObjString* string = allocate_object(sizeof(ObjString) + size + 1, OBJ_STRING);
//     string->size = size;
//     string->chars = chars;
//     return string;
// }

ObjString* make_string(uint32_t size)
{
    ObjString* string = (ObjString*)allocate_object(sizeof(ObjString) + size + 1, OBJ_STRING);
    string->size = size;
    return string;
}


ObjString* copy_string(const char* chars, uint32_t size)
{
    ObjString* string = make_string(size);
    memcpy(string->chars, chars, size);
    string->chars[size] = '\0';
    return string;
}


void print_object(Value value)
{
    switch(OBJ_TYPE(value))
    {
    case OBJ_STRING:
        printf("%s", AS_CSTRING(value));
        break;
    }
}