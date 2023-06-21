#include "slice.h"
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

typedef struct {
    size_t depth;
    size_t col;
    size_t ln;
    bool   in_jstring;
} skState;

static skState JsonState = {
    .ln         = 1,
    .col        = 1,
    .depth      = 1,
    .in_jstring = false,
};

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

int_least64_t
Sk_Slice_len(const Sk_StrSlice* slice)
{
    return (slice == NULL) ? -1 : (int_least64_t) slice->len;
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

char*
Sk_CharIter_next_address(const Sk_CharIter* iterator)
{
    if(iterator == NULL || iterator->next == NULL) {
        return NULL;
    }

    return iterator->next;
}

void
Sk_State_handle(char c)
{
    switch(c) {
        case '\n':
            if(!JsonState.in_jstring) {
                JsonState.col = 1;
                JsonState.ln++;
            } else {
                JsonState.col++;
            }
            break;
        case '{':
            JsonState.depth++;
            JsonState.col++;
            break;
        case '}':
            if(JsonState.depth > 1) {
                JsonState.depth--;
            }
            JsonState.col++;
            break;
        case '[':
            JsonState.depth++;
            JsonState.col++;
            break;
        case ']':
            JsonState.depth--;
            JsonState.col++;
            break;
        default:
            JsonState.col++;
            break;
    }
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

    Sk_State_handle(temp);

    return temp;
}

int
Sk_CharIter_advance(Sk_CharIter* iterator, size_t amount)
{
    if(iterator == NULL || iterator->next == NULL) {
        return EOF;
    }

    int c = Sk_CharIter_peek(iterator);

    while(amount--) {
        if((c = Sk_CharIter_next(iterator)) == EOF) {
            break;
        }
    }

    return c;
}

void
Sk_CharIter_depth_above(Sk_CharIter* iterator)
{
    if(JsonState.depth < 2) {
        Sk_CharIter_drain(iterator);
    } else {
        size_t target_depth = JsonState.depth - 1;

        while(JsonState.depth != target_depth) {
            Sk_CharIter_next(iterator);
        }
    }
}

void
Sk_CharIter_drain(Sk_CharIter* iterator)
{
    if(iterator != NULL) {
        iterator->next = NULL;
    }
}

int
Sk_CharIter_peek(const Sk_CharIter* iterator)
{
    if(iterator == NULL || iterator->next == NULL) {
        return EOF;
    }

    return *iterator->next;
}
