#ifdef SK_DBUG
#include <assert.h>
#endif
#include "skerror.h"
#include "skhashtable.h"
#include "skutils.h"
#include "skvec.h"
#include <stdlib.h>

#define has_key_destructor(table) (table) != NULL && (table)->free_key != NULL
#define has_val_destructor(table) (table) != NULL && (table)->free_val != NULL

/* Internal only -------------------------------------------------------*/
static const unsigned int PRIMES[] = {
    101,   199,   443,    881,    1759,   3517,   7069,    14143,
    28307, 56599, 113453, 228841, 465799, 982351, 1941601,
};

#define PSIZE sizeof(PRIMES) / sizeof(PRIMES[0])

typedef struct _skHashCell skHashCell;

static void*  _skHashTable_probe(const skHashTable* table, const void* key, bool* found);
static size_t _skHash_str(const char* str);
static double _skHashTable_load_factor(const skHashTable* table);
static bool   _skHashTable_expand(skHashTable* table);

static void _skHashCell_reset(skHashTable* table, skHashCell* cell);

/*---------------------------------------------------------------------*/

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
    skHashTable* table;
    skVec*       storage;

    if(is_null(cmp_key)) {
        THROW_ERR(MissingComparisonFn);
        return NULL;
    }

    table = malloc(sizeof(skHashTable));
    if(is_null(table)) {
        THROW_ERR(OutOfMemory);
        return NULL;
    }

    storage = skVec_new(sizeof(skHashCell));
    if(is_null(storage)) {
        free(table);
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
    return (double) table->len / (double) skVec_capacity(table->storage);
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

/* Expands the HashTable, if it fails the allocation it will
 print the err to stderr and return false. */
static bool
_skHashTable_expand(skHashTable* table)
{
    size_t         prime, i, old_cap, ele_size;
    skVec*         new_storage;
    skVec*         old_storage;
    unsigned char* old_allocation;
    skHashCell*    current;

    if(is_null(table)) {
        return false;
    }

    old_cap     = skVec_capacity(table->storage);
    ele_size    = skVec_element_size(table->storage);
    new_storage = NULL;

    /* Get the new table size and allocate new storage */
    for(i = 0; i < PSIZE; i++) {
        if((prime = PRIMES[i]) > old_cap) {
            new_storage = skVec_with_capacity(ele_size, prime);

            if(is_null(new_storage)) {
                THROW_ERR(OutOfMemory);
                return false;
            }

            break;
        }
    }

    /* Check if we exceeded max table size */
    if(is_null(new_storage)) {
        THROW_ERR(TableSizeLimit);
        return false;
    }

    /* Swap table storage (array) */
    old_storage    = table->storage;
    table->storage = new_storage;

    old_allocation = skVec_inner_unsafe(old_storage);
    current        = NULL;

    /* Re-hash all the keys */
    while(old_cap--) {
        if((current = (skHashCell*) old_allocation)->taken) {
#ifdef SK_DBUG
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
    assert(skVec_len(old_storage) == skVec_len(table->storage));
#endif

    /* Drop the boomer */
    skVec_drop(old_storage, NULL);
    return true;
}

void*
_skHashTable_probe(const skHashTable* table, const void* key, bool* found)
{
    skVec*      cells;
    size_t      storage_size;
    size_t      index;
    skHashCell* cell;
    int         i;

    *found       = false;
    cells        = table->storage;
    storage_size = skVec_capacity(table->storage);
    index        = (table->hash_fn(key)) % storage_size;
    cell         = skVec_index_unsafe(cells, index);

    for(i = 1; (!(*found) && cell->taken); i++) {
        if(table->cmp_key(cell->key, key) == 0) {
            *found = true;
        } else {
            /* Keys didn't match, keep probing */
            index = (index + (i * i)) % storage_size;
            cell  = skVec_index_unsafe(cells, index);
        }
    }

    return cell;
}

void*
skHashTable_get(const skHashTable* table, const void* key)
{
    skHashCell* cell;
    bool        found;

    if(is_null(table)) {
        return NULL;
    }

    if(is_null(key)) {
        THROW_ERR(InvalidKey);
        return NULL;
    }

    found = false;
    cell  = _skHashTable_probe(table, key, &found);

    return (found) ? cell->value : NULL;
}

/* TODO: Make table copy the key */
bool
skHashTable_insert(skHashTable* table, const void* key, const void* value)
{
    bool        found;
    skHashCell* cell;

    if(is_null(table)) {
        return false;
    }

    if(is_null(key)) {
        THROW_ERR(InvalidKey);
        return false;
    }

    if(is_null(value)) {
        THROW_ERR(InvalidValue);
        return false;
    }
#ifdef SK_DBUG
    assert(table->len <= skVec_capacity(table->storage));
#endif
    /* Expand if we exceeded load factor or the capacity is 0 */
    if(skVec_capacity(table->storage) == 0 || _skHashTable_load_factor(table) >= 0.5) {
        if(!_skHashTable_expand(table)) {
            return false;
        }
    }

    found = false;
    cell  = _skHashTable_probe(table, key, &found);

    if(!found) {
        cell->key   = discard_const(key);
        cell->taken = true;
        table->len++;
    } else if(has_val_destructor(table)) {
        table->free_val(cell->value);
    }

    cell->value = discard_const(value);

    return true;
}

bool
skHashTable_remove(skHashTable* table, void* key)
{
    skHashCell* cell;

    if(is_null(table)) {
        return false;
    }

    if(is_null(key)) {
        THROW_ERR(InvalidKey);
        return false;
    }

    cell = skHashTable_get(table, key);
    if(is_null(cell)) {
        return false;
    }

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
skHashTable_len(const skHashTable* table)
{
    if(is_null(table)) {
        return 0;
    }
    return table->len;
}

bool
skHashTable_contains(const skHashTable* table, const void* key)
{
    if(is_null(table)) {
        return false;
    }

    return (is_null(skHashTable_get(table, key))) ? false : true;
}

void
skHashTable_drop(skHashTable* table)
{
    size_t      size;
    skVec*      vec;
    skHashCell* cell;

    if(is_null(table)) {
        return;
    }

    size = skVec_capacity(table->storage);
    vec  = table->storage;

    while(size--) {
        if((cell = skVec_index_unsafe(vec, size))->taken) {
            _skHashCell_reset(table, cell);
        }
    }

    skVec_drop(table->storage, NULL);
    free(table);
}
