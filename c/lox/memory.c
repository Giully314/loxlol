/*
lox/memory.c
*/

#include "memory.h"
#include "common.h"
#include "object.h"
#include "vm.h"

#include <stdlib.h>


// old_size == 0 and new_size != 0       -> allocate new block.
// old_size != 0 and new_size == 0       -> free the block.
// old_size != 0 and new_size < old_size -> shrink .
// old_size != 0 and new_size > old_size -> grow.
void* reallocate(void* pointer, size_t old_size, size_t new_size)
{
    if (new_size == 0)
    {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, new_size);
    if (result == NULL)
    {
        exit(1);
    }

    return result;
}

static void free_object(Obj* obj)
{
    switch (obj->type)
    {
        case OBJ_STRING:
            ObjString* s = (ObjString*)obj;
            // FREE_ARRAY(char, s->chars, s->size + 1);
            // FREE(ObjString, obj);
            reallocate(s, sizeof(ObjString) + s->size + 1, 0);
            break;
    }
}

void free_objects()
{
    Obj* obj = vm.objects;
    while (obj != NULL)
    {
        Obj* next = obj->next;
        free_object(obj);
        obj = next;
    }
}