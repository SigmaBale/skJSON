#include "node.h"
#include "sk_vec.h"
#include <limits.h>
#include <memory.h>
#include <stdlib.h>

Sk_Vec
Sk_Vec_new(size_t ele_size)
{
    return (Sk_Vec) {
        .capacity   = 0,
        .len        = 0,
        .ele_size   = ele_size,
        .allocation = NULL,
    };
}

Sk_Vec
Sk_Vec_with_capacity(size_t ele_size, size_t capacity)
{
    void*  allocation = malloc(ele_size * capacity);
    Sk_Vec vec        = Sk_Vec_new(ele_size);

    if(allocation != NULL) {
        vec.allocation = allocation;
        vec.capacity   = capacity;
    } else {
        PRINT_OOM_ERR;
    }

    return vec;
}

static int
_Sk_Vec_maybe_grow(Sk_Vec* vec)
{
    if(vec->len == vec->capacity) {
        size_t amount;
        size_t cap = (vec->capacity == 0) ? 1 : vec->capacity * 2;

        if((amount = cap * vec->ele_size) > INT_MAX) {
            PRINT_ALLOC_TOO_BIG;
            return 1;
        }

        void* new_alloc = realloc(vec->allocation, amount);

        if(new_alloc == NULL) {
            PRINT_OOM_ERR;
            return 1;
        }

        vec->allocation = new_alloc;
        vec->capacity   = cap;
    }
    return 0;
}

void*
_Sk_Vec_get(Sk_Vec* vec, size_t index)
{
    return &vec->allocation[index * vec->ele_size];
}

void*
Sk_Vec_front(Sk_Vec* vec)
{
    if(vec == NULL || vec->len == 0) {
        return NULL;
    }

    return _Sk_Vec_get(vec, 0);
}

bool
Sk_Vec_push(Sk_Vec* vec, void* element)
{
    if(vec == NULL) {
        return false;
    }

    if(_Sk_Vec_maybe_grow(vec) == 1) {
        return false;
    }

    void* end = _Sk_Vec_get(vec, vec->len);
    memmove(end, element, vec->ele_size);

    vec->len++;

    return true;
}

void*
Sk_Vec_index(Sk_Vec* vec, size_t index)
{
    if(vec == NULL) {
        return NULL;
    }

    if(index >= vec->len) {
        PRINT_IDX_OOB;
        return NULL;
    }

    return _Sk_Vec_get(vec, index);
}

void*
Sk_Vec_pop(Sk_Vec* vec)
{
    if(vec == NULL || vec->len == 0) {
        return NULL;
    }

    return _Sk_Vec_get(vec, vec->len--);
}

int
Sk_Vec_len(Sk_Vec* vec)
{
    return (vec == NULL) ? -1 : vec->len;
}

static void
_Sk_Vec_drop_elements(Sk_Vec* vec, FreeFn free_fn)
{
    void* current;
    while((current = Sk_Vec_pop(vec)) != NULL) {
        free_fn(current);
    }
}

void
Sk_Vec_drop(Sk_Vec* vec, FreeFn free_fn)
{
    if(vec != NULL) {
        if(vec->allocation) {
            if(free_fn) {
                _Sk_Vec_drop_elements(vec, free_fn);
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
