/*

*/


#ifndef TABLE_H
#define TABLE_H


#include "common.h"
#include "value.h"
#include "object.h"


#define TABLE_MAX_LOAD 0.75

typedef struct
{
    ObjString* key;
    Value value;
} Entry;


typedef struct
{
    uint32_t size;
    uint32_t capacity;
    Entry* entries;
} HashTable;


void init_hashtable(HashTable* table);
void free_hashtable(HashTable* table);
bool set_hashtable(HashTable* table, ObjString* key, Value value);
bool get_hashtable(HashTable* table, ObjString* key, Value* value);
bool del_hashtable(HashTable* table, ObjString* key);
void add_all_hashtable(HashTable* from, HashTable* to);
ObjString* find_string_hashtable(HashTable* table, const char* chars, uint32_t size, uint32_t hash);


#endif