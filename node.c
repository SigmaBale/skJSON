#include "node.h"
#include <stdio.h>
#include <stdlib.h>

Sk_JsonNode*
Sk_JsonErrorNode_new(Sk_JsonError msg)
{
    Sk_JsonNode* node = malloc(sizeof(Sk_JsonNode));

    if(node == NULL) {
        PRINT_OOM_ERR;
        return NULL;
    }

    node->type       = SK_ERROR_NODE;
    node->data.j_err = msg;

    return node;
}

Sk_JsonNode*
Sk_JsonObjectNode_new(Sk_JsonMember** members, size_t len)
{
    Sk_JsonNode* node = malloc(sizeof(Sk_JsonNode));

    if(node == NULL) {
        PRINT_OOM_ERR;
        return NULL;
    }

    node->type                   = SK_OBJECT_NODE;
    node->data.j_object->members = members;
    node->data.j_object->len     = len;

    return node;
}

Sk_JsonNode*
Sk_JsonArrayNode_new(Sk_JsonNode** nodes, size_t len)
{
    Sk_JsonNode* node = malloc(sizeof(Sk_JsonNode));

    if(node == NULL) {
        PRINT_OOM_ERR;
        return NULL;
    }

    node->type                = SK_OBJECT_NODE;
    node->data.j_array->nodes = nodes;
    node->data.j_array->len   = len;

    return node;
}

Sk_JsonNode*
Sk_JsonStringNode_new(Sk_JsonString str)
{
    Sk_JsonNode* node = malloc(sizeof(Sk_JsonNode));

    if(node == NULL) {
        PRINT_OOM_ERR;
        return NULL;
    }

    node->type          = SK_STRING_NODE;
    node->data.j_string = str;

    return node;
}

Sk_JsonNode*
Sk_JsonIntegerNode_new(Sk_JsonInteger number)
{
    Sk_JsonNode* node = malloc(sizeof(Sk_JsonNode));

    if(node == NULL) {
        PRINT_OOM_ERR;
        return NULL;
    }

    node->type       = SK_INT_NODE;
    node->data.j_int = number;

    return node;
}

Sk_JsonNode*
Sk_JsonDoubleNode_new(Sk_JsonDouble number)
{
    Sk_JsonNode* node = malloc(sizeof(Sk_JsonNode));

    if(node == NULL) {
        PRINT_OOM_ERR;
        return NULL;
    }

    node->type          = SK_DOUBLE_NODE;
    node->data.j_double = number;

    return node;
}

Sk_JsonNode*
Sk_JsonBoolNode_new(Sk_JsonBool boolean)
{
    Sk_JsonNode* node = malloc(sizeof(Sk_JsonNode));

    if(node == NULL) {
        PRINT_OOM_ERR;
        return NULL;
    }

    node->type           = SK_BOOL_NODE;
    node->data.j_boolean = boolean;

    return node;
}

Sk_JsonNode*
Sk_JsonNullNode_new(void)
{
    Sk_JsonNode* node = malloc(sizeof(Sk_JsonNode));

    if(node == NULL) {
        PRINT_OOM_ERR;
        return NULL;
    }

    node->type        = SK_NULL_NODE;
    node->data.j_null = NULL;

    return node;
}

void
Sk_JsonObjectNode_drop(Sk_JsonObject* object)
{
    if(object != NULL) {
        size_t size = object->len;
        while(size--) {
            Sk_JsonMember_drop(*object->members++);
        }

        object->members = NULL;
        object->len     = 0;
    }
}

void
Sk_JsonArray_drop(Sk_JsonArray* array)
{
    if(array != NULL) {
        size_t size = array->len;
        while(size--) {
            Sk_JsonNode_drop(*array->nodes++);
        }

        array->nodes = NULL;
        array->len   = 0;
    }
}

void
Sk_JsonMember_drop(Sk_JsonMember* member)
{
    if(member != NULL) {
        free(member->string);
        Sk_JsonNode_drop(&member->value);
        member->string = NULL;
    }
}

void
Sk_JsonNode_drop(Sk_JsonNode* node)
{
    if(node != NULL) {
        switch(node->type) {
            case SK_OBJECT_NODE:
                Sk_JsonObject_drop(node->data.j_object);
                break;
            case SK_ARRAY_NODE:
                Sk_JsonArray_drop(node->data.j_array);
                break;
            case SK_STRING_NODE:
                free(node->data.j_string);
                break;
            default:
                break;
        }

        free(node);
    }
}
