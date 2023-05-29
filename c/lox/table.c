


#include "table.h"
#include "common.h"
#include "memory.h"
#include "object.h"
#include "value.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void init_hashtable(HashTable* table)
{
    table->capacity = 0;
    table->size = 0;
    table->entries = NULL;
}


void free_hashtable(HashTable* table)
{
    FREE_ARRAY(Entry, table->entries, table->capacity);
    init_hashtable(table);
}


static Entry* find_entry(Entry* entries, uint32_t capacity, ObjString* key)
{
    uint32_t idx = key->hash % capacity;
    Entry* tombstone = NULL;
    while (true)
    {
        Entry* entry = &entries[idx];

        // Check if the null value is a tombstone or a free entry.
        if (entry->key == NULL)
        {
            if (IS_NIL(entry->value)) 
            {
                return tombstone != NULL ? tombstone : entry; 
            }
            else
            {
                if (tombstone == NULL)
                {
                    tombstone = entry;
                }
            }
        }
        else if (entry->key == key)
        {
            return entry;
        }

        idx = (idx + 1) % capacity;
    }
}


static void adjust_capacity(HashTable* table, uint32_t capacity)
{
    Entry* entries = ALLOCATE(Entry, capacity);
    for (uint32_t i = 0; i < capacity; ++i)
    {
        entries[i].key = NULL;
        entries[i].value = NIL_VAL;
    }

    table->size = 0;
    for (uint32_t i = 0; i < table->capacity; ++i)
    {
        Entry* entry = &table->entries[i];
        if (entry->key == NULL)
        {
            continue;
        }

        Entry* dest = find_entry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        ++table->size;
    }
    
    FREE_ARRAY(Entry, table->entries, table->capacity);

    table->entries = entries;
    table->capacity = capacity;
}


bool set_hashtable(HashTable* table, ObjString* key, Value value)
{
    if (table->size + 1 > table->capacity * TABLE_MAX_LOAD)
    {
        uint32_t capacity = GROW_CAPACITY(table->capacity);
        adjust_capacity(table, capacity);
    }

    Entry* entry = find_entry(table->entries, table->capacity, key);
    bool is_new_key = entry->key == NULL;
    if (is_new_key && IS_NIL(entry->value))
    {
        ++table->size;
    }

    entry->key = key;
    entry->value = value;
    return is_new_key;
}


bool get_hashtable(HashTable* table, ObjString* key, Value* value)
{
    if (table->size == 0)
    {
        return false;
    }

    Entry* entry = find_entry(table->entries, table->capacity, key);
    if (entry->key == NULL)
    {
        return false;
    }

    *value = entry->value;
    return true;
}


bool del_hashtable(HashTable* table, ObjString* key)
{
    if (table->size == 0)
    {
        return false;
    }

    Entry* entry = find_entry(table->entries, table->capacity, key);
    if (entry->key == NULL)
    {
        return false;
    }

    // Place a tombstone.
    entry->key = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}


void add_all_hashtable(HashTable* from, HashTable* to)
{
    for (uint32_t i = 0; i < from->capacity; i++) 
    {
        Entry* entry = &from->entries[i];
        if (entry->key != NULL) 
        {
            set_hashtable(to, entry->key, entry->value);
        }
  }
}


ObjString* find_string_hashtable(HashTable* table, const char* chars, uint32_t size, uint32_t hash)
{
    if (table->size == 0)
    {
        return NULL;
    }

    uint32_t idx = hash % table->capacity;
    while (true)
    {
        Entry* entry = &table->entries[idx];

        if (entry->key == NULL)
        {
            // Empty non tombstone.
            if (IS_NIL(entry->value))
            {
                return NULL;  
            } 
        }
        else if (entry->key->size == size && entry->key->hash == hash && memcmp(entry->key->chars, chars, size) == 0)
        {
            // Found.
            return entry->key;
        }

        idx = (idx + 1) % table->capacity;
    }
}


void print_hashtable(HashTable* table)
{
    for (uint32_t i = 0; i < table->capacity; ++i)
    {
        Entry* entry = &table->entries[i];
        if (!IS_NIL(entry->value))
        {
            printf("Key: %s\n", entry->key->chars);
            printf("Value: ");
            print_value(entry->value);
            printf("\n\n");
        }
    }
}