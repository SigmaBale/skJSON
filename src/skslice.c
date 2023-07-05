#include "skslice.h"
#include "skutils.h"
#include <stdbool.h>

skStrSlice
skSlice_new(const char* ptr, size_t len)
{
    skStrSlice slice;
    slice.ptr = discard_const(ptr);
    slice.len = len;
    return slice;
}

char*
skSlice_start(const skStrSlice* slice)
{
    return (slice == NULL) ? NULL : slice->ptr;
}

char*
skSlice_end(const skStrSlice* slice)
{
    return (slice == NULL) ? NULL : slice->ptr + slice->len - 1;
}

char*
skSlice_index(const skStrSlice* slice, size_t index)
{
    char* temp;

    if(slice == NULL) {
        return NULL;
    }

    temp = slice->ptr + index;

    if(temp > slice->ptr + slice->len) {
        return NULL;
    }

    return temp;
}

long int
skSlice_len(const skStrSlice* slice)
{
    return (is_null(slice)) ? -1 : (long int) slice->len;
}

skJsonState
skJsonState_new(void)
{
    skJsonState state;
    state.ln         = 1;
    state.col        = 1;
    state.depth      = 1;
    state.in_jstring = false;
    return state;
}

skCharIter
skCharIter_new(const char* ptr, size_t len)
{
    skCharIter  iter;
    skJsonState state;

    state      = skJsonState_new();
    iter.next  = (char*) ptr;
    iter.end   = (char*) ptr + len;
    iter.state = state;

    return iter;
}

skCharIter
skCharIter_from_slice(skStrSlice* slice)
{
    skCharIter  iter;
    skJsonState state;

    state      = skJsonState_new();
    iter.next  = skSlice_start(slice);
    iter.end   = skSlice_end(slice);
    iter.state = state;

    return iter;
}

char*
skCharIter_next_address(const skCharIter* iterator)
{
    if(is_null(iterator) || is_null(iterator->next)) {
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
    char temp;

    if(is_null(iterator) || is_null(iterator->next)) {
        return EOF;
    }

    temp = *iterator->next;

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
    int c;

    if(is_null(iterator) || is_null(iterator->next)) {
        return EOF;
    }

    c = skCharIter_peek(iterator);

    while(amount--) {
        if((c = skCharIter_next(iterator)) == EOF) {
            break;
        }
    }

    return c;
}

/* Useless for now */
void
skCharIter_depth_above(skCharIter* iterator)
{
    size_t target_depth;

    if(iterator->state.depth < 2) {
        skCharIter_drain(iterator);
    } else {
        target_depth = iterator->state.depth - 1;

        while(iterator->state.depth != target_depth) {
            skCharIter_next(iterator);
        }
    }
}

void
skCharIter_drain(skCharIter* iterator)
{
    if(!is_null(iterator)) {
        iterator->next = NULL;
    }
}

int
skCharIter_peek(const skCharIter* iterator)
{
    if(is_null(iterator) || is_null(iterator->next)) {
        return EOF;
    }

    return *iterator->next;
}
