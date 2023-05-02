/*
lox/memory.h

PURPOSE:
    Provide utility function and macros for allocation/deallocation.
*/

#ifndef MEMORY_H
#define MEMORY_H

#include "common.h"

#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)


#define GROW_ARRAY(type, pointer, old_size, new_size) \
    (type*)reallocate(pointer, sizeof(type) * (old_size), \
    sizeof(type) * (new_size))


#define FREE_ARRAY(type, pointer, old_size) \
    reallocate(pointer, sizeof(type) * old_size, 0)


// old_size == 0 and new_size != 0       -> allocate new block.
// old_size != 0 and new_size == 0       -> free the block.
// old_size != 0 and new_size < old_size -> shrink .
// old_size != 0 and new_size > old_size -> grow.
void *reallocate(void *pointer, size_t old_size, size_t new_size);

#endif