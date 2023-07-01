#ifdef SK_DBUG
#include <assert.h>
#endif
#include "skerror.h"
#include "sknode.h"
#include "skutils.h"
#include <stdlib.h>
#include <string.h>

/* Err msg buffer size */
#define ERR_SIZE 100

skJsonNode*
RawNode_new(skNodeType type, skJsonNode* parent)
{
    skJsonNode* raw_node;

    raw_node = malloc(sizeof(skJsonNode));
    if(is_null(raw_node)) {
        THROW_ERR(OutOfMemory);
        return NULL;
    }

    raw_node->type   = type;
    raw_node->parent = parent;
    raw_node->index  = 0;

    return raw_node;
}

skJsonNode*
ObjectNode_new(skJsonNode* parent)
{
    skJsonNode* object_node;

    if(is_null(object_node = RawNode_new(SK_OBJECT_NODE, parent))) {
        return NULL;
    }

    object_node->data.j_object = skVec_new(sizeof(skObjectTuple));

    if(is_null(object_node->data.j_object)) {
        return NULL;
    }

    return object_node;
}

skJsonNode*
ArrayNode_new(skJsonNode* parent)
{
    skJsonNode* array_node;

    if(is_null(array_node = RawNode_new(SK_ARRAY_NODE, parent))) {
        return NULL;
    }

    if(is_null(array_node->data.j_array = skVec_new(sizeof(skJsonNode)))) {
        free(array_node);
        return NULL;
    }

    return array_node;
}

skJsonNode*
ErrorNode_new(skJsonString msg, skJsonState state, skJsonNode* parent)
{
    skJsonNode* node;
    char*       error;
    char        errmsg[ERR_SIZE];

    if(is_null(node = RawNode_new(SK_ERROR_NODE, parent))) {
        return NULL;
    }

    sprintf(
        errmsg,
        "%s: line %lu, col %lu, depth %lu\n",
        msg,
        state.ln,
        state.col,
        state.depth);

    if(is_null(error = strdup_ansi(errmsg))) {
        free(node);
        return NULL;
    }

    node->data.j_err = error;
    return node;
}

skJsonNode*
StringNode_new(const skJsonString str, skNodeType type, skJsonNode* parent)
{
    skJsonNode*  string_node;
    skJsonString dupped;

    if(type == SK_STRING_NODE) {
        if(is_null(dupped = strdup_ansi(str))) {
            return NULL;
        }
    } else {
        dupped = str;
    }

    if(is_null(string_node = RawNode_new(type, parent))) {
        if(type == SK_STRING_NODE) {
            free(dupped);
        }
        return NULL;
    }

    string_node->data.j_string = dupped;
    return string_node;
}

skJsonNode*
IntNode_new(long int n, skJsonNode* parent)
{
    skJsonNode* int_node;

    int_node = RawNode_new(SK_INT_NODE, parent);
    if(is_null(int_node)) {
        return NULL;
    }

    int_node->data.j_int = n;
    return int_node;
}

skJsonNode*
DoubleNode_new(double n, skJsonNode* parent)
{
    skJsonNode* double_node;

    double_node = RawNode_new(SK_DOUBLE_NODE, parent);
    if(is_null(double_node)) {
        return NULL;
    }

    double_node->data.j_double = n;
    return double_node;
}

skJsonNode*
BoolNode_new(bool boolean, skJsonNode* parent)
{
    skJsonNode* bool_node;

    bool_node = RawNode_new(SK_BOOL_NODE, parent);
    if(is_null(bool_node)) {
        return NULL;
    }

    bool_node->data.j_boolean = boolean;
    return bool_node;
}

void
skObjectTuple_drop(skObjectTuple* tuple)
{
#ifdef SK_DBUG
    assert(is_some(tuple));
    print_node(&tuple->value);
    assert(tuple->value.parent->type == SK_OBJECT_NODE);
#endif
    free(tuple->key);
    skJsonNode_drop(&tuple->value);
}

void
print_node(skJsonNode* node)
{
    if(is_null(node)) {
        printf("NULL");
    } else {
        switch(node->type) {
            case SK_STRINGLIT_NODE:
            case SK_STRING_NODE:
                printf("String Node -> '%s', parent: ", node->data.j_string);
                print_node(node->parent);
                break;
            case SK_INT_NODE:
                printf("Int Node -> '%ld', parent: ", node->data.j_int);
                print_node(node->parent);
                break;
            case SK_DOUBLE_NODE:
                printf("Double Node -> '%fl', parent: ", node->data.j_double);
                print_node(node->parent);
                break;
            case SK_BOOL_NODE:
                printf(
                    "Bool Node -> '%s', parent: ",
                    (node->data.j_boolean) ? "true" : "false");
                print_node(node->parent);
                break;
            case SK_NULL_NODE:
                printf("Null Node -> NULL, parent: ");
                print_node(node->parent);
                break;
            case SK_ARRAY_NODE:
                printf("Array Node -> len '%ld', parent: ", skVec_len(node->data.j_array));
                print_node(node->parent);
                break;
            case SK_OBJECT_NODE:
                printf("Object Node -> len '%ld', parent: ", skVec_len(node->data.j_object));
                print_node(node->parent);
                break;
            case SK_ERROR_NODE:
                printf("Error Node -> '%s', parent: ", node->data.j_err);
                print_node(node->parent);
                break;
            default:
                printf("%u ...", node->type);
                printf("Unreachable");
        }
    }
    printf("\n");
}

void
skJsonNode_drop(skJsonNode* node)
{
    skJsonNode* parent;

    if(!is_null(node)) {
        switch(node->type) {
            case SK_OBJECT_NODE:
                skVec_drop(node->data.j_object, (FreeFn) skObjectTuple_drop);
                break;
            case SK_ARRAY_NODE:
                skVec_drop(node->data.j_array, (FreeFn) skJsonNode_drop);
                break;
            case SK_STRING_NODE:
                free(node->data.j_string);
                break;
            case SK_ERROR_NODE:
                free(node->data.j_err);
            default:
                break;
        }

        parent = node->parent;
        if(is_some(parent)) {
            if(parent->type == SK_ARRAY_NODE) {
                skVec_remove(parent->data.j_array, node->index, NULL);
            } else {
#ifdef SK_DBUG
                assert(parent->type == SK_OBJECT_NODE);
#endif
                skVec_remove(parent->data.j_object, node->index, NULL);
            }

        } else {
            free(node);
        }
    }
}
