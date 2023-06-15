#include "slice.h"
#include <stdio.h>

StrSlice
slice_new(char* ptr, size_t len)
{
    return (StrSlice) {
        .ptr = ptr,
        .len = len,
    };
}

char*
slice_start(const StrSlice* slice)
{
    return (slice == NULL) ? NULL : slice->ptr;
}

char*
slice_end(const StrSlice* slice)
{
    return (slice == NULL) ? NULL : slice->ptr + slice->len;
}

char*
slice_index(const StrSlice* slice, size_t index)
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
slice_len(const StrSlice* slice)
{
    return (slice == NULL) ? -1 : slice->len;
}

CharIterator
iterator_char_new(const char* ptr, size_t len)
{
    return (CharIterator) {
        .next = (char*) ptr,
        .end  = (char*) ptr + len - 1,
    };
}

CharIterator
iterator_char_from_slice(StrSlice* slice)
{
    return (CharIterator) { .next = slice_start(slice), .end = slice_end(slice) };
}

int
iterator_char_next(CharIterator* iterator)
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
iterator_char_peek_next(const CharIterator* iterator)
{
    if(iterator == NULL || iterator->next == NULL) {
        return EOF;
    }

    return *iterator->next;
}

int
iterator_char_peek(const CharIterator* iterator, size_t offset)
{
    if(iterator == NULL || iterator->next == NULL || iterator->next + offset > iterator->end) {
        return EOF;
    }

    return *(iterator->next + offset);
}
