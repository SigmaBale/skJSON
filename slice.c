#include "slice.h"
#include <stdio.h>

Sk_StrSlice
Sk_Slice_new(char* ptr, size_t len)
{
    return (Sk_StrSlice) {
        .ptr = ptr,
        .len = len,
    };
}

char*
Sk_Slice_start(const Sk_StrSlice* slice)
{
    return (slice == NULL) ? NULL : slice->ptr;
}

char*
Sk_Slice_end(const Sk_StrSlice* slice)
{
    return (slice == NULL) ? NULL : slice->ptr + slice->len;
}

char*
Sk_Slice_index(const Sk_StrSlice* slice, size_t index)
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
Sk_Slice_len(const Sk_StrSlice* slice)
{
    return (slice == NULL) ? -1 : slice->len;
}

Sk_CharIter
Sk_CharIter_new(const char* ptr, size_t len)
{
    return (Sk_CharIter) {
        .next = (char*) ptr,
        .end  = (char*) ptr + len,
    };
}

Sk_CharIter
Sk_CharIter_from_slice(Sk_StrSlice* slice)
{
    return (Sk_CharIter) { .next = Sk_Slice_start(slice), .end = Sk_Slice_end(slice) };
}

int
Sk_CharIter_next(Sk_CharIter* iterator)
{
    if(iterator == NULL || iterator->next == NULL) {
        return EOF;
    }

    char temp = *iterator->next;

    if(iterator->next == iterator->end) {
        iterator->next = NULL;
    } else {
        iterator->next++;
    }

    return temp;
}

int
Sk_CharIter_peek_next(const Sk_CharIter* iterator)
{
    if(iterator == NULL || iterator->next == NULL) {
        return EOF;
    }

    return *iterator->next;
}

int
Sk_CharIter_peek(const Sk_CharIter* iterator, size_t offset)
{
    if(iterator == NULL || iterator->next == NULL || iterator->next + offset > iterator->end) {
        return EOF;
    }

    return *(iterator->next + offset);
}
