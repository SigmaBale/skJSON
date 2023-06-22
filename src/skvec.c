// clang-format off
#include <limits.h>
#include <stdbool.h>
#include <memory.h>
#include <stdlib.h>
#include "skvec.h"
#include "sknode.h"

// clang-format on

skVec*
skVec_new(size_t ele_size)
{
    skVec* vec = malloc(sizeof(skVec));
    null_check_oom_err(vec);

    vec->ele_size   = ele_size;
    vec->capacity   = 0;
    vec->len        = 0;
    vec->allocation = NULL;

    return vec;
}

skVec*
skVec_with_capacity(size_t ele_size, size_t capacity)
{
    skVec* vec = skVec_new(ele_size);
    null_check_with_ret(vec, NULL);

    void* allocation = calloc(capacity, ele_size);
    null_check_with_err_and_ret(allocation, PRINT_OOM_ERR, NULL);

    vec->allocation = allocation;
    vec->capacity   = capacity;

    return vec;
}

static int
_skVec_maybe_grow(skVec* vec)
{
    if(vec->len == vec->capacity) {
        size_t amount;
        size_t cap = (vec->capacity == 0) ? 1 : vec->capacity * 2;

        if(__glibc_unlikely((amount = cap * vec->ele_size) > INT_MAX)) {
            print_err_and_ret(PRINT_ALLOC_TOO_BIG, 1);
        }

        void* new_alloc = realloc(vec->allocation, amount);
        null_check_with_err_and_ret(new_alloc, PRINT_OOM_ERR, 1);

        vec->allocation = new_alloc;
        vec->capacity   = cap;
    }

    return 0;
}

void*
_skVec_get(skVec* vec, size_t index)
{
    return &vec->allocation[index * vec->ele_size];
}

void*
skVec_index_unsafe(skVec* vec, size_t index)
{
    null_check_with_ret(vec, NULL);
    check_with_err_and_ret(index >= vec->capacity, PRINT_IDX_OOB, NULL);
    return _skVec_get(vec, index);
}

bool
skVec_insert_non_contiguous(skVec* vec, void* element, size_t index)
{
    null_check_with_ret(vec, false);
    null_check_with_ret(element, false);
    check_with_err_and_ret(index >= vec->capacity, PRINT_IDX_OOB, false);

    void* hole = _skVec_get(vec, index);
    memmove(hole, element, vec->ele_size);

    return true;
}

void*
skVec_front(skVec* vec)
{
    check_with_ret((vec == NULL || vec->len == 0), NULL);
    return _skVec_get(vec, 0);
}

void*
skVec_back(skVec* vec)
{
    check_with_ret((vec == NULL || vec->len == 0), NULL);
    return _skVec_get(vec, vec->len - 1);
}

bool
skVec_push(skVec* vec, void* element)
{
    null_check_with_ret(vec, false);
    check_with_ret(__glibc_unlikely(_skVec_maybe_grow(vec) == 1), false);

    memmove(_skVec_get(vec, vec->len), element, vec->ele_size);
    vec->len++;

    return true;
}

void*
skVec_index(skVec* vec, size_t index)
{
    null_check_with_ret(vec, NULL);
    check_with_err_and_ret(index >= vec->len, PRINT_IDX_OOB, NULL);
    return _skVec_get(vec, index);
}

void*
skVec_pop(skVec* vec)
{

    check_with_ret(vec == NULL || vec->len == 0, NULL);
    return _skVec_get(vec, --vec->len);
}

int
skVec_len(skVec* vec)
{
    null_check_with_ret(vec, -1);
    return (int_least64_t) vec->len;
}

bool
skVec_insert(skVec* vec, void* element, size_t index)
{
    null_check_with_ret(vec, false);
    null_check_with_err_and_ret(element, INVALID_KEY, false);
    check_with_err_and_ret(index > vec->len, PRINT_IDX_OOB, false);

    if(__glibc_unlikely(_skVec_maybe_grow(vec) == 1)) {
        return false;
    }

    if(index == vec->len) {
        if(__glibc_unlikely(skVec_push(vec, element))) {
            return false;
        }
    } else {
        void*  hole   = _skVec_get(vec, index + 1);
        size_t elsize = vec->ele_size;

        memmove(hole, hole - elsize, (vec->len - index) * elsize);
        memcpy(_skVec_get(vec, index), element, elsize);
        vec->len++;
    }

    return true;
}

bool
skVec_remove(skVec* vec, size_t index, FreeFn free_fn)
{
    null_check_with_ret(vec, false);
    check_with_err_and_ret(index >= vec->len, PRINT_IDX_OOB, false);

    void* hole = _skVec_get(vec, index);

    if(free_fn) {
        free_fn(hole);
    }

    size_t elsize = vec->ele_size;
    memmove(hole, hole + elsize, (--vec->len - index) * elsize);

    return true;
}

static void
_skVec_drop_elements(skVec* vec, FreeFn free_fn)
{
    void* current;
    while((current = skVec_pop(vec)) != NULL) {
        free_fn(current);
    }
}

void
skVec_drop(skVec* vec, FreeFn free_fn)
{
    if(vec != NULL) {
        if(vec->allocation != NULL) {
            if(free_fn) {
                _skVec_drop_elements(vec, free_fn);
            }
            free(vec->allocation);
        }
        vec->allocation = NULL;
        vec->capacity   = 0;
        vec->len        = 0;
        vec->ele_size   = 0;
        free(vec);
    }
}
