#ifndef __SK_HASHTABLE_H__
#define __SK_HASHTABLE_H__

#include "sktypes.h"
#include <stddef.h>

typedef size_t (*HashFn)(const void *);

typedef int (*CmpKeyFn)(const void *, const void *);

typedef void (*FreeKeyFn)(void *);

typedef void (*FreeValueFn)(void *);

typedef struct skHashTable skHashTable;

typedef struct skTableIter skTableIter;

typedef struct skTuple skTuple;

struct skTuple {
  const void *key;
  const void *value;
};

skHashTable *skHashTable_new(HashFn hash_fn, CmpKeyFn cmp_key,
                             FreeKeyFn free_key, FreeValueFn free_val);

bool skHashTable_insert(skHashTable *table, const void *key, const void *value);

void *skHashTable_get(const skHashTable *table, const void *key);

bool skHashTable_contains(const skHashTable *table, const void *key);

size_t skHashTable_len(const skHashTable *table);

bool skHashTable_remove(skHashTable *table, void *key);

void skHashTable_drop(skHashTable *table);

skTableIter *skHashTable_into_iter(const skHashTable *table);

skTuple skTableIter_next(skTableIter *iter);

void skTableIter_drop(skTableIter *iter);

#endif
