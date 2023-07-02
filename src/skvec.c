#include "skerror.h"
#include "skutils.h"
#include "skvec.h"
#include <assert.h>
#include <limits.h>
#include <memory.h>
#include <stdlib.h>

static void* _skVec_get(const skVec* vec, const size_t index);

struct skVec {
    unsigned char* allocation;
    size_t         ele_size;
    size_t         capacity;
    size_t         len;
};

skVec*
skVec_new(const size_t ele_size)
{
    skVec* vec;

    vec = malloc(sizeof(skVec));
    if(is_null(vec)) {
        THROW_ERR(OutOfMemory);
        return NULL;
    }

    vec->ele_size   = ele_size;
    vec->capacity   = 0;
    vec->len        = 0;
    vec->allocation = NULL;

    return vec;
}

skVec*
skVec_with_capacity(const size_t ele_size, const size_t capacity)
{
    skVec* vec;
    void*  allocation;

    vec = skVec_new(ele_size);
    if(is_null(vec)) {
        return NULL;
    }

    allocation = calloc(capacity, ele_size);
    if(is_null(allocation)) {
        THROW_ERR(OutOfMemory);
        return NULL;
    }

    vec->allocation = allocation;
    vec->capacity   = capacity;

    return vec;
}

static int
_skVec_maybe_grow(skVec* vec)
{
    size_t amount;
    size_t cap;
    void*  new_alloc;

    if(vec->len == vec->capacity) {
        cap = (vec->capacity == 0) ? 10 : vec->capacity * 2;

        if((amount = cap * vec->ele_size) > INT_MAX) {
            THROW_ERR(AllocationTooLarge);
            return 1;
        }

        new_alloc = realloc(vec->allocation, amount);
        if(is_null(new_alloc)) {
            THROW_ERR(OutOfMemory);
            return 1;
        }

        vec->allocation = new_alloc;
        vec->capacity   = cap;
    }

    return 0;
}

static void*
_skVec_get(const skVec* vec, const size_t index)
{
    return (vec->allocation + (index * vec->ele_size));
}

void*
skVec_index_unsafe(const skVec* vec, const size_t index)
{
    if(is_null(vec)) {
        return NULL;
    }

    if(index >= vec->capacity) {
        THROW_ERR(IndexOutOfBounds);
        return NULL;
    }

    return _skVec_get(vec, index);
}

bool
skVec_insert_non_contiguous(skVec* vec, const void* element, const size_t index)
{
    void* hole;

    if(is_null(vec) || is_null(element)) {
        return false;
    }

    if(index >= vec->capacity) {
        THROW_ERR(IndexOutOfBounds);
        return false;
    }

    hole = _skVec_get(vec, index);
    memmove(hole, element, vec->ele_size);

    return true;
}

void*
skVec_front(const skVec* vec)
{
    if(is_null(vec) || vec->len == 0) {
        return NULL;
    }

    return _skVec_get(vec, 0);
}

void*
skVec_inner_unsafe(const skVec* vec)
{
    if(is_null(vec)) {
        return NULL;
    }
    return vec->allocation;
}

void*
skVec_back(const skVec* vec)
{
    if(is_null(vec) || vec->len == 0) {
        return NULL;
    }

    return _skVec_get(vec, vec->len - 1);
}

bool
skVec_push(skVec* vec, const void* element)
{
    if(is_null(vec)) {
        return false;
    }

    if(_skVec_maybe_grow(vec) == 1) {
        return false;
    }

    memmove(_skVec_get(vec, vec->len), element, vec->ele_size);
    vec->len++;

    return true;
}

void
skVec_clear(skVec* vec, FreeFn free_fn)
{
    void* current;

    if(is_null(vec) || is_null(vec->allocation)) {
        return;
    }

    if(free_fn) {
        /* TODO: Fix json to stop relying on not using pop */
        while(is_some(current = skVec_pop(vec))) {
            free_fn(current);
        }
    }

    free(vec->allocation);
    vec->allocation = NULL;
    vec->capacity   = 0;
    vec->len        = 0;
}

bool
skVec_contains(const skVec* vec, const void* key, CmpFn cmp, bool sorted)
{
    return is_some(skVec_get_by_key(vec, key, cmp, sorted));
}

void*
skVec_get_by_key(const skVec* vec, const void* key, CmpFn cmp, bool sorted)
{
    void*  current;
    size_t size;

    if(is_null(vec)) {
        return NULL;
    }

    if(is_null(key)) {
        THROW_ERR(InvalidKey);
        return NULL;
    }

    if(is_null(cmp)) {
        THROW_ERR(MissingComparisonFn);
        return NULL;
    }

    if(sorted) {
        current = bsearch(key, vec->allocation, vec->len, vec->ele_size, cmp);
        return (is_some(current)) ? current : NULL;
    } else {
        size = vec->len;
        while(size--) {
            current = _skVec_get(vec, size);
            if(cmp(current, key) == 0) {
                return current;
            }
        }
    }

    return NULL;
}

bool
skVec_sort(skVec* vec, CmpFn cmp)
{
    if(is_null(vec)) {
        return false;
    }

    if(is_null(cmp)) {
        THROW_ERR(MissingComparisonFn);
        return false;
    }

    qsort(vec->allocation, vec->len, vec->ele_size, cmp);
    return true;
}

bool
skVec_remove_by_key(skVec* vec, const void* key, CmpFn cmp, FreeFn free_fn, bool sorted)
{
    void*  current;
    size_t size, idx;

    if(is_null(vec)) {
        return false;
    }

    if(is_null(key)) {
        THROW_ERR(InvalidKey);
        return false;
    }

    if(is_null(cmp)) {
        THROW_ERR(MissingComparisonFn);
        return false;
    }

    if(sorted) {
        current = bsearch(key, vec->allocation, vec->len, vec->ele_size, cmp);
        if(is_some(current)) {
            idx = ((unsigned char*) current - vec->allocation) / vec->ele_size;
            return skVec_remove(vec, idx, free_fn);
        }
    } else {
        size = vec->len;
        while(size--) {
            current = _skVec_get(vec, size);
            if(cmp(current, key) == 0) {
                return skVec_remove(vec, size, free_fn);
            }
        }
    }

    return false;
}

void*
skVec_index(const skVec* vec, const size_t index)
{
    if(is_null(vec)) {
        return NULL;
    }

    if(index >= vec->len) {
        THROW_ERR(IndexOutOfBounds);
        return NULL;
    }

    return _skVec_get(vec, index);
}

void*
skVec_pop(skVec* vec)
{
    void* retval;

    if(is_null(vec) || vec->len == 0) {
        return NULL;
    }

    retval = malloc(vec->ele_size);
    if(is_null(retval)) {
        THROW_ERR(OutOfMemory);
        return NULL;
    }

    memcpy(retval, _skVec_get(vec, --vec->len), vec->ele_size);
    return retval;
}

size_t
skVec_len(const skVec* vec)
{
    if(is_null(vec)) {
        return 0;
    }
    return vec->len;
}

size_t
skVec_capacity(const skVec* vec)
{
    if(is_null(vec)) {
        return 0;
    }
    return vec->capacity;
}

size_t
skVec_element_size(const skVec* vec)
{
    if(is_null(vec)) {
        return 0;
    }
    return vec->ele_size;
}

bool
skVec_insert(skVec* vec, const void* element, const size_t index)
{
    unsigned char* hole;
    size_t         elsize;

    if(is_null(vec) || is_null(element)) {
        return false;
    }

    if(index > vec->len) {
        THROW_ERR(IndexOutOfBounds);
        return false;
    }

    if(_skVec_maybe_grow(vec) == 1) {
        return false;
    }

    if(index == vec->len) {
        if(!skVec_push(vec, element)) {
            return false;
        }
    } else {
        hole   = _skVec_get(vec, index + 1);
        elsize = vec->ele_size;

        memmove(hole, hole - elsize, (vec->len - index) * elsize);
        memcpy(_skVec_get(vec, index), element, elsize);
        vec->len++;
    }

    return true;
}

bool
skVec_remove(skVec* vec, const size_t index, FreeFn free_fn)
{
    unsigned char* hole;
    size_t         elsize;

    if(is_null(vec)) {
        return false;
    }

    if(index >= vec->len) {
        THROW_ERR(IndexOutOfBounds);
        return false;
    }

    hole = _skVec_get(vec, index);

    if(free_fn) {
        free_fn(hole);
    }

    elsize = vec->ele_size;
    memmove(hole, hole + elsize, (--vec->len - index) * elsize);
    return true;
}

static void
_skVec_drop_elements(skVec* vec, FreeFn free_fn)
{
    size_t size;

    size = vec->len;
    while(size--) {
        free_fn(_skVec_get(vec, size));
    }
}

void
skVec_drop(skVec* vec, FreeFn free_fn)
{
    if(is_null(vec)) {
        return;
    }

    if(is_some(vec->allocation)) {
        if(is_some(free_fn)) {
            _skVec_drop_elements(vec, free_fn);
#ifdef SK_DBUG
            assert(vec->len == 0);
#endif
        }
        free(vec->allocation);
    }

    vec->allocation = NULL;
    vec->capacity   = 0;
    vec->ele_size   = 0;
    vec->len        = 0;
    free(vec);
}
