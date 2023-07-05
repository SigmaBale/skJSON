#ifdef SK_DBUG
#include <assert.h>
#endif
#include "skerror.h"
#include "sknode.h"
#include "skutils.h"
#include <stdlib.h>
#include <string.h>

/* Err msg buffer size */
#define ERR_SIZE 200

skJson RawNode_new(skNodeType type, const skJson* parent)
{
    skJson raw_node;
    raw_node.type = type;

    if(is_some(parent)) {
        /* We could use the same field 'j_array' in both cases it is a union
         * after all and both the array and object use the vector as arena.
         * Keep it seperated for future maintenance and readability. */
        if(parent->type == SK_ARRAY_NODE) {
            raw_node.parent_arena.ptr = (void*) parent->data.j_array;
        } else {
            raw_node.parent_arena.ptr = (void*) parent->data.j_object;
        }
        raw_node.parent_arena.type = parent->type;
    } else {
        raw_node.parent_arena.ptr  = NULL;
        raw_node.parent_arena.type = SK_NONE_NODE;
    }

    return raw_node;
}

skJson ObjectNode_new(const skJson* parent)
{
    skJson object_node;
    object_node = RawNode_new(SK_OBJECT_NODE, parent);

    if(is_null(object_node.data.j_object = skVec_new(sizeof(skObjTuple)))) {
        object_node.type = SK_ERROR_NODE;
    }

    return object_node;
}

skJson ArrayNode_new(const skJson* parent)
{
    skJson array_node;
    array_node = RawNode_new(SK_ARRAY_NODE, discard_const(parent));

    if(is_null(array_node.data.j_array = skVec_new(sizeof(skJson)))) {
        array_node.type = SK_ERROR_NODE;
    }

    return array_node;
}

skJson ErrorNode_new(const skJsonString msg, skJsonState state, const skJson* parent)
{
    skJson err_node;
    char   errmsg[ERR_SIZE];

    err_node = RawNode_new(SK_ERROR_NODE, discard_const(parent));

    sprintf(
        errmsg,
        "%s: line %lu, col %lu, depth %lu\n",
        msg,
        state.ln,
        state.col,
        state.depth);

    err_node.data.j_err = strdup_ansi(errmsg);
    return err_node;
}

skJson StringNode_new(const skJsonString str, skNodeType type, const skJson* parent)
{
    skJson string_node;
    string_node = RawNode_new(type, discard_const(parent));

    if(type == SK_STRING_NODE) {
        if(is_null(string_node.data.j_string = strdup_ansi(str))) {
            string_node.type = SK_ERROR_NODE;
            return string_node;
        }
    } else {
        string_node.data.j_string = str;
    }

    string_node.type = type;
    return string_node;
}

skJson IntNode_new(long int n, const skJson* parent)
{
    skJson int_node;
    int_node            = RawNode_new(SK_INT_NODE, discard_const(parent));
    int_node.data.j_int = n;
    return int_node;
}

skJson DoubleNode_new(double n, const skJson* parent)
{
    skJson double_node;
    double_node               = RawNode_new(SK_DOUBLE_NODE, discard_const(parent));
    double_node.data.j_double = n;
    return double_node;
}

skJson BoolNode_new(bool boolean, const skJson* parent)
{
    skJson bool_node;
    bool_node                = RawNode_new(SK_BOOL_NODE, discard_const(parent));
    bool_node.data.j_boolean = boolean;
    return bool_node;
}

void skObjTuple_drop(skObjTuple* tuple)
{
    free(tuple->key);
    skJsonNode_drop(&tuple->value);
}

void skJsonNode_drop(skJson* node)
{
    if(is_some(node)) {
#ifdef SK_DBUG
        assert(node->type != SK_DROPPED_NODE);
#endif
        switch(node->type) {
            case SK_OBJECT_NODE:
                skVec_drop(node->data.j_object, (FreeFn) skObjTuple_drop);
                node->data.j_object = NULL;
                break;
            case SK_ARRAY_NODE:
                skVec_drop(node->data.j_array, (FreeFn) skJsonNode_drop);
                node->data.j_array = NULL;
                break;
            case SK_STRING_NODE:
                free(node->data.j_string);
                break;
            case SK_ERROR_NODE:
                free(node->data.j_err);
                break;
            default:
                break;
        }
    }
}
