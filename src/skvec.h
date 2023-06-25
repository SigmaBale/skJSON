#ifndef __SK_VEC_H__
#define __SK_VEC_H__

#include <stdio.h>

#define PRINT_IDX_OOB fprintf(stderr, "Index out of bounds!\n");
#define PRINT_ALLOC_TOO_BIG fprintf(stderr, "Allocation is too big");

typedef struct skVec skVec;

struct skVec {
  unsigned char *allocation;
  size_t ele_size;
  size_t capacity;
  size_t len;
};

typedef void (*FreeFn)(void *);

skVec *skVec_new(size_t ele_size);

skVec *skVec_with_capacity(size_t ele_size, size_t capacity);

bool skVec_push(skVec *vec, void *element);

void *skVec_pop(skVec *vec);

size_t skVec_len(skVec *vec);

size_t skVec_capacity(skVec *vec);

void skVec_drop(skVec *vec, FreeFn free_fn);

void *skVec_index(skVec *vec, size_t index);

void *skVec_front(skVec *vec);

void *skVec_back(skVec *vec);

bool skVec_insert_non_contiguous(skVec *vec, void *element, size_t index);

void *skVec_index_unsafe(skVec *vec, size_t index);

bool skVec_insert(skVec *vec, void *element, size_t index);

bool skVec_remove(skVec *vec, size_t index, FreeFn free_fn);

#endif
