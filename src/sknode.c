// clang-format off
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sknode.h"

// clang-format on

skJsonNode*
skJsonNode_default(skJsonNode* parent)
{
    skJsonNode* node = malloc(sizeof(skJsonNode));
    null_check_with_err_and_ret(node, PRINT_OOM_ERR, NULL);

    node->parent = parent;
    node->data   = (skNodeData) { 0 };

    return node;
}

skJsonNode*
skJsonObject_new(skJsonNode* parent)
{
    skJsonNode* object_node = skJsonNode_default(parent);
    null_check_with_ret(object_node, NULL);

    object_node->data.j_object = skHashTable_new(NULL,
                                                 (FreeKeyFn) free,
                                                 (FreeValueFn) skJsonNode_drop);

    null_check_with_err_and_ret(object_node->data.j_object,
                                PRINT_OOM_ERR,
                                NULL);

    object_node->type = SK_OBJECT_NODE;
    return object_node;
}

skJsonNode*
skJsonArray_new(skJsonNode* parent)
{
    skJsonNode* array_node = skJsonNode_default(parent);
    null_check_with_ret(array_node, NULL);

    array_node->data.j_array = skVec_new(sizeof(skJsonMember));
    null_check_with_err_and_ret(array_node->data.j_array, PRINT_OOM_ERR, NULL);

    array_node->type = SK_ARRAY_NODE;
    return array_node;
}

/// TODO: Add State struct information such as column and line
/// number to the error message, or even make skJsonError a struct
skJsonNode*
skJsonError_new(skJsonError msg, skJsonNode* parent)
{
    skJsonNode* node = skJsonNode_default(parent);
    null_check_with_ret(node, NULL);

    node->type       = SK_ERROR_NODE;
    node->data.j_err = msg;

    return node;
}

skJsonNode*
skJsonStringNode_new(skJsonString str, skJsonNode* parent)
{
    skJsonNode* node = skJsonNode_default(parent);
    null_check_with_ret(node, NULL);

    node->type = (strlen(str) == 0) ? SK_EMPTYSTRING_NODE : SK_STRING_NODE;
    node->data.j_string = str;

    return node;
}

skJsonNode*
skJsonInteger_new(skJsonInteger number, skJsonNode* parent)
{
    skJsonNode* node = skJsonNode_default(parent);
    null_check_with_ret(node, NULL);

    node->type       = SK_INT_NODE;
    node->data.j_int = number;

    return node;
}

skJsonNode*
skJsonDouble_new(skJsonDouble number, skJsonNode* parent)
{
    skJsonNode* node = skJsonNode_default(parent);
    null_check_with_ret(node, NULL);

    node->type          = SK_DOUBLE_NODE;
    node->data.j_double = number;

    return node;
}

skJsonNode*
skJsonBoolNode_new(skJsonBool boolean, skJsonNode* parent)
{
    skJsonNode* node = skJsonNode_default(parent);
    null_check_with_ret(node, NULL);

    node->type           = SK_BOOL_NODE;
    node->data.j_boolean = boolean;

    return node;
}

skJsonNode*
skJsonNull_new(skJsonNode* parent)
{
    skJsonNode* node = skJsonNode_default(parent);
    null_check_with_ret(node, NULL);

    node->type = SK_NULL_NODE;

    return node;
}

void
skJsonNode_drop(skJsonNode* node)
{
    if(node != NULL) {
        print_node(node);
        switch(node->type) {
            case SK_OBJECT_NODE:
                skHashTable_drop(node->data.j_object);
                break;
            case SK_ARRAY_NODE:
                skVec_drop(node->data.j_array, (FreeFn) skJsonNode_drop);
                break;
            case SK_STRING_NODE:
                free(node->data.j_string);
                break;
            default:
                break;
        }

        skJsonNode* parent;
        if((parent = node->parent) != NULL) {
            if(parent->type == SK_OBJECT_NODE) {
                /// Node parent is Json Object, lets remove it from that object
                skHashTable_remove(parent->data.j_object, node->data.j_string);
            } else if(parent->type == SK_ARRAY_NODE) {
                /// Node parent is Json Array, lets remove it from that array
                skVec_remove(parent->data.j_array,
                             node->index,
                             (FreeFn) skJsonNode_drop);
            }
        } else {
            /// Node is root, we can free it directly
            free(node);
        }
    }
}
