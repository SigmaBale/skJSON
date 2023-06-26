#ifndef __SK_HASHTABLE_H__
#define __SK_HASHTABLE_H__

#include <stddef.h>
#include "sktypes.h"

typedef size_t (*HashFn)(const void *);

typedef int (*CmpKeyFn)(const void *, const void *);

typedef void (*FreeKeyFn)(void *);

typedef void (*FreeValueFn)(void *);

typedef struct skHashTable skHashTable;

skHashTable *skHashTable_new(HashFn hash_fn, CmpKeyFn cmp_key,
                             FreeKeyFn free_key, FreeValueFn free_val);

bool skHashTable_insert(skHashTable *table, void *key, void *value);

void *skHashTable_get(const skHashTable *table, void *key);

bool skHashTable_contains(const skHashTable *table, void *key);

size_t skHashTable_len(const skHashTable *table);

bool skHashTable_remove(skHashTable *table, void *key);

void skHashTable_drop(skHashTable *table);

#endif
