// clang-format off
#include <stdbool.h>
#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include <sys/types.h>
#include "sknode.h"
#include "skhashtable.h"
#include "skvec.h"
// clang-format on

#define has_key_destructor(table) (table) != NULL && (table)->free_key != NULL
#define has_val_destructor(table) (table) != NULL && (table)->free_val != NULL

/// Internal only -------------------------------------------------------///
static const unsigned int PRIMES[] = {
    101,   199,   443,    881,    1759,   3517,   7069,    14143,
    28307, 56599, 113453, 228841, 465799, 982351, 1941601,
};

#define PSIZE sizeof(PRIMES) / sizeof(PRIMES[0])

typedef struct _skHashCell skHashCell;

static void* _skHashTable_probe(skHashTable* table, void* key, bool* found);
static long unsigned int _skHash_str(const char* str);
static double            _skHashTable_load_factor(const skHashTable* table);
static bool              _skHashTable_expand(skHashTable* table);

static void _skHashCell_reset(skHashTable* table, skHashCell* cell);

///---------------------------------------------------------------------///

struct _skHashCell {
    void* key;
    void* value;
    bool  taken;
};

struct skHashTable {
    skVec*      storage;
    size_t      len;
    HashFn      hash_fn;
    CmpKeyFn    cmp_key;
    FreeKeyFn   free_key;
    FreeValueFn free_val;
};

skHashTable*
skHashTable_new(
    HashFn      hash_fn,
    CmpKeyFn    cmp_key,
    FreeKeyFn   free_key,
    FreeValueFn free_val)
{
    null_check_with_ret(cmp_key, NULL);

    skHashTable* table = malloc(sizeof(skHashTable));

    if(__glibc_unlikely(is_null(table))) {
        return NULL;
    }

    skVec* storage = skVec_new(sizeof(skHashCell));

    if(__glibc_unlikely(is_null(storage))) {
        free(table);
        PRINT_OOM_ERR;
        return NULL;
    }

    table->storage  = storage;
    table->cmp_key  = cmp_key;
    table->free_key = free_key;
    table->free_val = free_val;
    table->len      = 0;

    if(is_null(hash_fn)) {
        table->hash_fn = (HashFn) _skHash_str;
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
            break;
        }
    }

    /// Check if we exceeded max table size
    if(__glibc_unlikely(is_null(new_storage))) {
        TABLE_SIZE_LIMIT;
        return false;
    }

    /// Swap table storage (array)
    skVec* old_storage = table->storage;
    table->storage     = new_storage;

    unsigned char* old_allocation = old_storage->allocation;
    skHashCell*    current        = NULL;

    /// Re-hash all the keys
    while(old_cap--) {
        if((current = (skHashCell*) old_allocation)->taken) {
#ifdef sk_dbug
            if(!skHashTable_insert(table, current->key, current->value)) {
                assert(false);
            }
#else
            skHashTable_insert(table, current->key, current->value);
#endif
        }
        old_allocation += ele_size;
    }

#ifdef SK_DBUG
    assert(old_storage->len == table->storage->len);
#endif

    /// Drop the boomer
    skVec_drop(old_storage, NULL);
    return true;
}

void*
_skHashTable_probe(skHashTable* table, void* key, bool* found)
{
    *found                   = false;
    skVec*      cells        = table->storage;
    size_t      storage_size = table->storage->capacity;
    size_t      index        = (table->hash_fn(key)) % storage_size;
    skHashCell* cell         = skVec_index_unsafe(cells, index);

    for(int i = 1; (!(*found) && cell->taken); i++) {
        if(table->cmp_key(cell->key, key) == 0) {
            *found = true;
        } else {
            /// Keys didn't match, keep probing
            index = (index + (i * i)) % storage_size;
            cell  = skVec_index_unsafe(cells, index);
        }
    }

    return cell;
}

void*
skHashTable_get(skHashTable* table, void* key)
{
    check_with_ret(is_null(table) || is_null(key), NULL);
    bool        found = false;
    skHashCell* cell  = _skHashTable_probe(table, key, &found);
    return (found) ? cell->value : NULL;
}

bool
skHashTable_insert(skHashTable* table, void* key, void* value)
{
    check_with_ret(table == NULL || key == NULL, false);
#ifdef SK_DBUG
    assert(table->len <= table->storage->capacity);
#endif
    /// Expand if we exceeded load factor or the capacity is 0
    if(skVec_capacity(table->storage) == 0
       || _skHashTable_load_factor(table) >= 0.5)
    {
        check_with_ret(__glibc_unlikely(!_skHashTable_expand(table)), false);
    }

    /// Check if the table contains the cell with that key
    bool        found = false;
    skHashCell* cell  = _skHashTable_probe(table, key, &found);

    if(!found) {
        cell->key   = key;
        cell->taken = true;
        table->len++;
    } else if(has_val_destructor(table)) {
        table->free_val(cell->value);
    }

    cell->value = value;

    return true;
}

bool
skHashTable_remove(skHashTable* table, void* key)
{
    if(is_null(table) || is_null(key)) {
        assert(false);
        return false;
    }

    skHashCell* cell = skHashTable_get(table, key);
    null_check_with_ret(cell, false);

    _skHashCell_reset(table, cell);
    return true;
}

static void
_skHashCell_reset(skHashTable* table, skHashCell* cell)
{
    if(has_key_destructor(table)) {
        table->free_key(cell->key);
    }
    if(has_val_destructor(table)) {
        table->free_val(cell->value);
    }
    cell->key   = NULL;
    cell->value = NULL;
    cell->taken = false;
}

size_t
skHashTable_len(skHashTable* table)
{
    null_check_with_ret(table, 0);
    return table->len;
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
    size_t      size = table->storage->capacity;
    skVec*      vec  = table->storage;
    skHashCell* cell;

    while(size--) {
        if((cell = skVec_index_unsafe(vec, size))->taken) {
            _skHashCell_reset(table, cell);
        }
    }

    skVec_drop(table->storage, NULL);
    free(table);
}
