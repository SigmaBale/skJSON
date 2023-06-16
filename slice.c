#include "slice.h"
#include <stdio.h>

Sk_StrSlice
slice_new(char* ptr, size_t len)
{
    return (Sk_StrSlice) {
        .ptr = ptr,
        .len = len,
    };
}

char*
slice_start(const Sk_StrSlice* slice)
{
    return (slice == NULL) ? NULL : slice->ptr;
}

char*
slice_end(const Sk_StrSlice* slice)
{
    return (slice == NULL) ? NULL : slice->ptr + slice->len;
}

char*
slice_index(const Sk_StrSlice* slice, size_t index)
{
    if(slice == NULL) {
        return NULL;
    }

    char* temp = slice->ptr + index;

    if(temp > slice->ptr + slice->len) {
        return NULL;
    }

    return temp;
}

long int
slice_len(const Sk_StrSlice* slice)
{
    return (slice == NULL) ? -1 : slice->len;
}

Sk_CharIter
iterator_char_new(const char* ptr, size_t len)
{
    return (Sk_CharIter) {
        .next = (char*) ptr,
        .end  = (char*) ptr + len - 1,
    };
}

Sk_CharIter
iterator_char_from_slice(Sk_StrSlice* slice)
{
    return (Sk_CharIter) { .next = slice_start(slice), .end = slice_end(slice) };
}

int
iterator_char_next(Sk_CharIter* iterator)
{
    if(iterator == NULL || iterator->next == NULL) {
        return EOF;
    }

    char temp = *iterator->next;

    if(iterator->next == iterator->end) {
        iterator->next = NULL;
    }

    iterator->next++;
    return temp;
}

int
iterator_char_peek_next(const Sk_CharIter* iterator)
{
    if(iterator == NULL || iterator->next == NULL) {
        return EOF;
    }

    return *iterator->next;
}

int
iterator_char_peek(const Sk_CharIter* iterator, size_t offset)
{
    if(iterator == NULL || iterator->next == NULL || iterator->next + offset > iterator->end) {
        return EOF;
    }

    return *(iterator->next + offset);
}
