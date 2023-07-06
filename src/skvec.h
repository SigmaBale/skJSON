#ifndef __SK_VEC_H__
#define __SK_VEC_H__

#include "sktypes.h"
#include <stdio.h>

typedef struct skVec skVec;

skVec *skVec_new(const size_t ele_size);

skVec *skVec_with_capacity(const size_t ele_size, const size_t capacity);

bool skVec_push(skVec *vec, const void *element);

bool skVec_pop(skVec *vec, void* dst);

size_t skVec_len(const skVec *vec);

size_t skVec_capacity(const skVec *vec);

size_t skVec_element_size(const skVec *vec);

bool skVec_contains(const skVec *vec, const void *key, CmpFn cmp, bool sorted);

void skVec_clear(skVec *vec, FreeFn free_fn);

void *skVec_get_by_key(const skVec *vec, const void *key, CmpFn cmp,
                       bool sorted);

bool skVec_remove_by_key(skVec *vec, const void *key, CmpFn cmp, FreeFn free_fn,
                         bool sorted);

bool skVec_sort(skVec *vec, CmpFn cmp);

bool skVec_is_sorted(skVec* vec, CmpFn cmp);

void skVec_drop(skVec *vec, FreeFn free_fn);

void *skVec_index(const skVec *vec, const size_t index);

void *skVec_front(const skVec *vec);

void *skVec_inner_unsafe(const skVec *vec);

void *skVec_back(const skVec *vec);

bool skVec_insert_non_contiguous(skVec *vec, const void *element,
                                 const size_t index);

void *skVec_index_unsafe(const skVec *vec, const size_t index);

bool skVec_insert(skVec *vec, const void *element, const size_t index);

bool skVec_remove(skVec *vec, const size_t index, FreeFn free_fn);

#endif
