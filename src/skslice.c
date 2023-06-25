#include "skslice.h"
#include <stdbool.h>
#include <stdint.h>

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

long int
skSlice_len(const skStrSlice* slice)
{
    return (slice == NULL) ? -1 : (long int) slice->len;
}

skCharIter
skCharIter_new(const char* ptr, size_t len)
{
    return (skCharIter) {
        .next = (char*) ptr,
        .end  = (char*) ptr + len,
        .state
        = (skJsonState) {.ln = 1, .col = 1, .depth = 1, .in_jstring = false},
    };
}

skCharIter
skCharIter_from_slice(skStrSlice* slice)
{
    return (skCharIter) { .next = skSlice_start(slice),
                          .end  = skSlice_end(slice) };
}

char*
skCharIter_next_address(const skCharIter* iterator)
{
    if(iterator == NULL || iterator->next == NULL) {
        return NULL;
    }

    return iterator->next;
}

static void
skCharIter_update_state(skCharIter* iterator, char ch)
{

    switch(ch) {
        case '{':
            if(!iterator->state.in_jstring) {
                iterator->state.depth++;
            }
            iterator->state.col++;
            break;
        case '}':
            if(!iterator->state.in_jstring) {
                iterator->state.depth--;
            }
            iterator->state.col++;
            break;
        case '[':
            if(!iterator->state.in_jstring) {
                iterator->state.depth++;
            }
            iterator->state.col++;
            break;
        case ']':
            if(!iterator->state.in_jstring) {
                iterator->state.depth--;
            }
            iterator->state.col++;
            break;
        case '\n':
            if(!iterator->state.in_jstring) {
                iterator->state.ln++;
                iterator->state.col = 1;
            } else {
                iterator->state.col++;
            }
            break;
        default:
            iterator->state.col++;
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

    skCharIter_update_state(iterator, temp);
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
    if(iterator->state.depth < 2) {
        skCharIter_drain(iterator);
    } else {
        size_t target_depth = iterator->state.depth - 1;

        while(iterator->state.depth != target_depth) {
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
