#ifndef __SK_HASHTABLE_H__
#define __SK_HASHTABLE_H__

#include <stdbool.h>
#include <stddef.h>

typedef size_t (*HashFunction)(const void *);

typedef int (*CmpKeyFn)(const void *, const void *);

typedef void (*FreeKeyFn)(void *);

typedef void (*FreeValueFn)(void *);

typedef struct skHashTable skHashTable;

skHashTable *skHashTable_new(HashFunction hash_fn, CmpKeyFn cmp_key,
                             FreeKeyFn free_key, FreeValueFn free_val);

bool skHashTable_insert(skHashTable *table, void *key, void *value);

void *skHashTable_get(skHashTable *table, void *key);

bool skHashTable_contains(skHashTable *table, void *key);

bool skHashTable_remove(skHashTable *table, void *key);

void skHashTable_drop(skHashTable *table);

#endif
