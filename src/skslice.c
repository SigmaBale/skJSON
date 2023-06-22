#include "skslice.h"
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

struct _skState {
    size_t depth;
    size_t col;
    size_t ln;
    bool   in_jstring;
};

/// TODO: Make JsonState part of the skJson struct
/// Must avoid having global, what if we have multiple json
/// parsed files in a single proccess?
skState JsonState = {
    .ln         = 1,
    .col        = 1,
    .depth      = 1,
    .in_jstring = false,
};

skStrSlice
skSlice_new(char* ptr, size_t len)
{
    return (skStrSlice) {
        .ptr = ptr,
        .len = len,
    };
}

char*
skSlice_start(const skStrSlice* slice)
{
    return (slice == NULL) ? NULL : slice->ptr;
}

char*
skSlice_end(const skStrSlice* slice)
{
    return (slice == NULL) ? NULL : slice->ptr + slice->len;
}

char*
skSlice_index(const skStrSlice* slice, size_t index)
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
skSlice_len(const skStrSlice* slice)
{
    return (slice == NULL) ? -1 : (int_least64_t) slice->len;
}

skCharIter
skCharIter_new(const char* ptr, size_t len)
{
    return (skCharIter) {
        .next = (char*) ptr,
        .end  = (char*) ptr + len,
    };
}

skCharIter
skCharIter_from_slice(skStrSlice* slice)
{
    return (skCharIter) { .next = skSlice_start(slice), .end = skSlice_end(slice) };
}

char*
skCharIter_next_address(const skCharIter* iterator)
{
    if(iterator == NULL || iterator->next == NULL) {
        return NULL;
    }

    return iterator->next;
}

void
skState_handle(char c)
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
skCharIter_next(skCharIter* iterator)
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

    skState_handle(temp);

    return temp;
}

int
skCharIter_advance(skCharIter* iterator, size_t amount)
{
    if(iterator == NULL || iterator->next == NULL) {
        return EOF;
    }

    int c = skCharIter_peek(iterator);

    while(amount--) {
        if((c = skCharIter_next(iterator)) == EOF) {
            break;
        }
    }

    return c;
}

void
skCharIter_depth_above(skCharIter* iterator)
{
    if(JsonState.depth < 2) {
        skCharIter_drain(iterator);
    } else {
        size_t target_depth = JsonState.depth - 1;

        while(JsonState.depth != target_depth) {
            skCharIter_next(iterator);
        }
    }
}

void
skCharIter_drain(skCharIter* iterator)
{
    if(iterator != NULL) {
        iterator->next = NULL;
    }
}

int
skCharIter_peek(const skCharIter* iterator)
{
    if(iterator == NULL || iterator->next == NULL) {
        return EOF;
    }

    return *iterator->next;
}
