// clang-format off
#include <stdbool.h>
#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include "sknode.h"
#include "skhashtable.h"
// clang-format on

#define is_some(object) (object) != NULL

static const unsigned int PRIMES[] = {
    101,   199,   443,    881,    1759,   3517,   7069,    14143,
    28307, 56599, 113453, 228841, 465799, 982351, 1941601,
};

#define PSIZE sizeof(PRIMES) / sizeof(PRIMES[0])

typedef struct _skHashCell skHashCell;

static long unsigned int _skHash_str(const char* str);
static double            _skHashTable_load_factor(const skHashTable* table);
static bool              _skHashTable_expand(skHashTable* table);
static skHashCell*       _skHashCell_new(void*       key,
                                         void*       value,
                                         FreeKeyFn   free_key,
                                         FreeValueFn free_val);
static void              _skHashCell_drop(skHashCell* cell);

struct _skHashCell {
    void*       key;
    void*       value;
    bool        taken;
    FreeKeyFn   free_key;
    FreeValueFn free_val;
};

struct skHashTable {
    skVec*       storage;
    size_t       len;
    HashFunction hash_fn;
    FreeKeyFn    free_key;
    FreeValueFn  free_val;
};

static skHashCell*
_skHashCell_new(void*       key,
                void*       value,
                FreeKeyFn   free_key,
                FreeValueFn free_val)
{
    null_check_with_err_and_ret(key, INVALID_KEY, NULL);

    skHashCell* cell = malloc(sizeof(skHashCell));

    if(__glibc_unlikely(is_null(cell))) {
        PRINT_OOM_ERR;
        return NULL;
    }

    cell->key      = key;
    cell->value    = value;
    cell->free_key = free_key;
    cell->free_val = free_val;

    return cell;
}

skHashTable*
skHashTable_new(HashFunction hash_fn, FreeKeyFn free_key, FreeValueFn free_val)
{
    skHashTable* table = malloc(sizeof(skHashTable));

    if(__glibc_unlikely(is_null(table))) {
        return NULL;
    }

    skVec* storage = skVec_new(sizeof(skHashCell*));

    if(__glibc_unlikely(is_null(storage))) {
        free(table);
        PRINT_OOM_ERR;
        return NULL;
    }

    table->storage  = storage;
    table->free_key = free_key;
    table->free_val = free_val;
    table->len      = 0;

    if(is_null(hash_fn)) {
        table->hash_fn = (HashFunction) _skHash_str;
    } else {
        table->hash_fn = hash_fn;
    }

    return table;
}

/* Returns the current load factor. */
static double
_skHashTable_load_factor(const skHashTable* table)
{
    return (double) table->len / table->storage->capacity;
}

/* Default hashing function */
/* http://www.cse.yorku.ca/~oz/hash.html */
static size_t
_skHash_str(const char* str)
{
    static const size_t HASHCONST = 5381;
    size_t              hash      = HASHCONST;
    int                 c;

    while((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

/// Expands the HashTable, if it fails the allocation it will
/// print the err to stderr and return false.
///
static bool
_skHashTable_expand(skHashTable* table)
{
    null_check_with_ret(table, false);

    size_t old_cap     = table->storage->capacity;
    size_t ele_size    = table->storage->ele_size;
    skVec* new_storage = NULL;

    size_t prime;
    /// Get the new table size and allocate new storage
    for(size_t i = 0; i < PSIZE; i++) {
        if((prime = PRIMES[i]) > old_cap) {
            new_storage = skVec_with_capacity(ele_size, prime);

            if(__glibc_unlikely(is_null(new_storage))) {
                PRINT_OOM_ERR;
                return false;
            }

            new_storage->capacity = prime;
        }
    }

    /// Check if we exceeded max table size
    if(__glibc_unlikely(is_null(new_storage))) {
        TABLE_SIZE_LIMIT;
        return false;
    }

    /// Swap table storage
    skVec* old_storage = table->storage;
    table->storage     = new_storage;

    size_t      old_len        = old_storage->len;
    void*       old_allocation = old_storage->allocation;
    skHashCell* current        = NULL;

    /// Re-hash all the keys
    while(old_cap--) {
        if((current = old_allocation++)->taken) {
            /// No need to check for this, we already allocated the new table,
            /// so we guarrantee it won't try to expand, meaning this can't
            /// error (aka return false). But we check for the return value
            /// anyways because why not...
            if(__glibc_unlikely(
                   !skHashTable_insert(table, current->key, current->value)))
            {
                free(old_allocation);
                return false;
            }
        }
    }
#ifdef DBUG
    assert(old_len == table->storage->len);
#endif
    return true;
}

void*
skHashTable_get(skHashTable* table, void* key)
{
    check_with_ret(table == NULL || key == NULL, NULL);

    skVec*      cells = table->storage;
    size_t      index = table->hash_fn(key) % cells->capacity;
    skHashCell* cell  = skVec_index_unsafe(cells, index);

    return (cell->taken) ? cell->value : NULL;
}

bool
skHashTable_insert(skHashTable* table, void* key, void* value)
{
    check_with_ret(table == NULL || key == NULL, false);

    void* entry = skHashTable_get(table, key);

    if(is_some(entry)) {
        memmove(entry, value, sizeof(uintptr_t));
        return true;
    }

#ifdef DBUG
    assert(table->len <= table->storage->capacity);
#endif

    /// Expand if we exceeded load factor
    if(_skHashTable_load_factor(table) >= 0.5) {
        check_with_ret(__glibc_unlikely(!_skHashTable_expand(table)), false);
    }

    size_t      storage_size = table->storage->capacity;
    skVec*      cells        = table->storage;
    size_t      index        = table->hash_fn(key) % storage_size;
    skHashCell* cell         = skVec_index_unsafe(cells, index);

    for(int i = 1; cell && !cell->taken; i++) {
        index = (index + (i * i)) % storage_size;
        cell  = skVec_index_unsafe(cells, index);
    }

    memmove(cell->key, key, sizeof(uintptr_t));
    memmove(cell->value, value, sizeof(uintptr_t));
    cell->taken = true;

    if(__glibc_unlikely(!skVec_insert_non_contiguous(cells, cell, index))) {
        return false;
    }

    table->len++;

    return true;
}

bool
skHashTable_remove(skHashTable* table, void* key)
{
    if(is_null(table) || is_null(table)) {
        return false;
    }

    skHashCell* cell = skHashTable_get(table, key);
    null_check_with_ret(cell, false);

    _skHashCell_drop(cell);
    return true;
}

bool
skHashTable_contains(skHashTable* table, void* key)
{
    null_check_with_ret(table, false);
    return (skHashTable_get(table, key) == NULL) ? false : true;
}

void
skHashTable_drop(skHashTable* table)
{
    if(is_null(table)) {
        return;
    }

    skVec_drop(table->storage, (FreeFn) _skHashCell_drop);
    free(table);
}

static void
_skHashCell_drop(skHashCell* cell)
{
    if(is_null(cell)) {
        return;
    }

    if(cell->free_key) {
        cell->free_key(cell->key);
    }

    if(cell->free_val) {
        cell->free_val(cell->value);
    }

    cell->taken = false;
    free(cell);
}
