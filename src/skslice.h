#ifndef __SK_SLICE_H__
#define __SK_SLICE_H__

#include <stddef.h>
#include <stdio.h>

/// TODO: Implement proper API
typedef struct _skState skState;

/**
 * 'StrSlice' is a contigious sequence of bytes.
 */
typedef struct {
  char *ptr;
  size_t len;
} skStrSlice;

/**
 * 'StrSlice' constructor, PTR denotes the start of sequence,
 * LEN is the number of elements in the sequence.
 */
skStrSlice skSlice_new(char *ptr, size_t len);

/**
 * Returns the pointer to the current/starting byte of SLICE.
 * Returns NULL if SLICE is NULL or if starting byte is NULL.
 */
char *skSlice_start(const skStrSlice *slice);

/**
 * Returns the pointer to the last byte of SLICE.
 * Returns NULL if SLICE is NULL or if last byte is NULL.
 */
char *skSlice_end(const skStrSlice *slice);

/**
 * Returns a pointer to the element at the INDEX or
 * NULL if INDEX is out of bounds and/or SLICE is NULL.
 */
char *skSlice_index(const skStrSlice *slice, size_t index);

/**
 * Returns the SLICE length.
 * Returns -1 if SLICE is NULL.
 */
long int skSlice_len(const skStrSlice *slice);

/**
 * Iterator over char's (bytes).
 */
typedef struct {
  char *next;
  char *end;
} skCharIter;

/**
 * 'CharIterator' constructor.
 * PTR is starting byte of iterator and LEN the len of iterator.
 */
skCharIter skCharIter_new(const char *ptr, size_t len);

/**
 * Constructs the 'CharIterator' from the 'StrSlice'.
 * Returns NULL if SLICE is NULL.
 */
skCharIter skCharIter_from_slice(skStrSlice *slice);

/**
 * Returns the next char from ITERATOR, if ITERATOR is exhausted
 * (was already at the end) or ITERATOR is NULL then it returns EOF.
 */
int skCharIter_next(skCharIter *iterator);

/**
 * Peeks at the next char without advancing the iterator (consuming
 * the value).
 * Returns either a char that we peeked at or EOF is ITERATOR is NULL
 * or ITERATOR is exhausted.
 */
int skCharIter_peek(const skCharIter *iterator);

/**
 * Returns pointer to the current value the ITERATOR is pointing at.
 */
char *skCharIter_next_address(const skCharIter *iterator);

int skCharIter_advance(skCharIter *iterator, size_t amount);

void skCharIter_drain(skCharIter *iterator);

void skCharIter_depth_above(skCharIter *iterator);

#endif
