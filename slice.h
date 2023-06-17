#ifndef __SK_SLICE_H__
#define __SK_SLICE_H__

#include <stddef.h>

/**
 * 'StrSlice' is a contigious sequence of bytes.
 * Contigious means that every element is the
 * same distance from its neighbour/s.
 */
typedef struct {
  char *ptr;
  size_t len;
} Sk_StrSlice;

/**
 * 'StrSlice' constructor, PTR denotes the start of sequence,
 * LEN is the length of the sequence.
 */
Sk_StrSlice Sk_Slice_new(char *ptr, size_t len);

/**
 * Returns the pointer to the current/starting byte of SLICE.
 * Returns NULL if SLICE is NULL or if starting byte is NULL.
 */
char *Sk_Slice_start(const Sk_StrSlice *slice);

/**
 * Returns the pointer to the last byte of SLICE.
 * Returns NULL if SLICE is NULL or if last byte is NULL.
 */
char *Sk_Slice_end(const Sk_StrSlice *slice);

/**
 * Returns a pointer to the element at the INDEX or
 * NULL if INDEX is out of bounds and/or SLICE is NULL.
 */
char *Sk_Slice_index(const Sk_StrSlice *slice, size_t index);

/**
 * Returns the SLICE length.
 * Returns -1 if SLICE is NULL.
 */
long int Sk_Slice_len(const Sk_StrSlice *slice);

/**
 * Iterator over char's (bytes).
 */
typedef struct {
  char *next;
  char *end;
} Sk_CharIter;

/**
 * 'CharIterator' constructor.
 * PTR is starting byte of iterator and LEN the len of iterator.
 */
Sk_CharIter Sk_CharIter_new(const char *ptr, size_t len);

/**
 * Constructs the 'CharIterator' from the 'StrSlice'.
 * Returns NULL if SLICE is NULL.
 */
Sk_CharIter Sk_CharIter_from_slice(Sk_StrSlice *slice);

/**
 * Returns the next char from ITERATOR, if ITERATOR is exhausted
 * (was already at the end) or ITERATOR is NULL then it returns EOF.
 */
int Sk_CharIter_next(Sk_CharIter *iterator);

/**
 * Peeks at the next char without advancing the iterator (consuming
 * the value).
 * Returns either a char that we peeked at or EOF is ITERATOR is NULL
 * or ITERATOR is exhausted.
 */
int Sk_CharIter_peek_next(const Sk_CharIter *iterator);

/**
 * Peeks at the next ITERATOR value with OFFSET without advancing the
 * iterator.
 * If OFFSET and current ITERATOR position are bigger than the ITERATOR'S
 * length or ITERATOR is NULL or ITERATOR is exhausted, then it returns EOF.
 */
int Sk_CharIter_peek(const Sk_CharIter *iterator, size_t offset);

#endif
