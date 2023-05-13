
#include "object.h"
#include "memory.h"
#include "value.h"
#include "common.h"
#include "vm.h"
#include "table.h"

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


static uint32_t hash_string(const char* key, uint32_t size)
{
    uint32_t hash = 2166136261u;
    for (uint32_t i = 0; i < size; ++i)
    {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}


ObjString* make_string(uint32_t size)
{
    ObjString* string = (ObjString*)allocate_object(sizeof(ObjString) + size + 1, OBJ_STRING);
    string->size = size;
    return string;
}


ObjString* concatenate_string(ObjString* s1, ObjString* s2)
{
    uint32_t size = s1->size + s2->size;
    ObjString* result = make_string(size);
    memcpy(result->chars, s1->chars, s1->size);
    memcpy(result->chars + s1->size, s2->chars, s2->size);
    result->chars[size] = '\0';
    result->hash = hash_string(result->chars, result->size);
    
    // Probability to have a string already registered that is equal to the concatenation of 2 strings is low.
    // I don't like this approach, could be a waste of resources.
    ObjString* interned = find_string_hashtable(&vm.strings, result->chars, result->size, result->hash);
    if (interned != NULL)
    {
        // Free the previous allocated string.
        free_object((Obj*)result);
        return interned;
    } 
    
    set_hashtable(&vm.strings, result, NIL_VAL);
    return result;
}


ObjString* copy_string(const char* chars, uint32_t size)
{   
    uint32_t hash = hash_string(chars, size);
    
    // Check if the string is already registered.
    ObjString* interned = find_string_hashtable(&vm.strings, chars, size, hash);
    if (interned != NULL)
    {
        return interned;
    } 

    ObjString* string = make_string(size);
    memcpy(string->chars, chars, size);
    string->chars[size] = '\0';
    string->hash = hash;
    set_hashtable(&vm.strings, string, NIL_VAL);
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