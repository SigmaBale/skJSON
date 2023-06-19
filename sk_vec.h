#ifndef __SK_VEC_H__
#define __SK_VEC_H__

#include <stdio.h>

#define PRINT_IDX_OOB fprintf(stderr, "Index out of bounds!\n");
#define PRINT_ALLOC_TOO_BIG fprintf(stderr, "Allocation is too big");

typedef struct Sk_Vec Sk_Vec;

struct Sk_Vec {
  void *allocation;
  size_t ele_size;
  size_t capacity;
  size_t len;
};

typedef void (*FreeFn)(void *);

Sk_Vec Sk_Vec_new(size_t ele_size);

Sk_Vec Sk_Vec_with_capacity(size_t ele_size, size_t capacity);

bool Sk_Vec_push(Sk_Vec *vec, void *element);

void *Sk_Vec_pop(Sk_Vec *vec);

int Sk_Vec_len(Sk_Vec *vec);

void Sk_Vec_drop(Sk_Vec *vec, FreeFn free_fn);

void *Sk_Vec_index(Sk_Vec *vec, size_t index);

void *Sk_Vec_front(Sk_Vec *vec);

void *Sk_Vec_back(Sk_Vec *vec);

#endif
