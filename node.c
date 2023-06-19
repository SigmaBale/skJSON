#include "node.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Sk_JsonNode*
Sk_JsonNode_default(void)
{
    Sk_JsonNode* node = malloc(sizeof(Sk_JsonNode));

    if(node == NULL) {
        PRINT_OOM_ERR;
    }

    return node;
}

Sk_JsonNode*
Sk_JsonErrorNode_new(Sk_JsonError msg)
{
    Sk_JsonNode* node = Sk_JsonNode_default();

    if(node == NULL) {
        return NULL;
    }

    node->type        = SK_ERROR_NODE;
    node->data->j_err = msg;

    return node;
}

Sk_JsonNode*
Sk_JsonObjectNode_new(Sk_Vec members)
{
    Sk_JsonNode* node = Sk_JsonNode_default();

    if(node == NULL) {
        return NULL;
    }

    node->type                   = SK_OBJECT_NODE;
    node->data->j_object.members = members;

    return node;
}

Sk_JsonNode*
Sk_JsonArrayNode_new(Sk_Vec nodes)
{
    Sk_JsonNode* node = Sk_JsonNode_default();

    if(node == NULL) {
        return NULL;
    }

    node->type                = SK_ARRAY_NODE;
    node->data->j_array.nodes = nodes;

    return node;
}

Sk_JsonNode*
Sk_JsonStringNode_new(Sk_JsonString str)
{
    Sk_JsonNode* node = Sk_JsonNode_default();

    if(node == NULL) {
        return NULL;
    }

    node->type           = (strlen(str) == 0) ? SK_EMPTYSTRING_NODE : SK_STRING_NODE;
    node->data->j_string = str;

    return node;
}

Sk_JsonNode*
Sk_JsonIntegerNode_new(Sk_JsonInteger number)
{
    Sk_JsonNode* node = Sk_JsonNode_default();

    if(node == NULL) {
        return NULL;
    }

    node->type        = SK_INT_NODE;
    node->data->j_int = number;

    return node;
}

Sk_JsonNode*
Sk_JsonDoubleNode_new(Sk_JsonDouble number)
{
    Sk_JsonNode* node = Sk_JsonNode_default();

    if(node == NULL) {
        return NULL;
    }

    node->type           = SK_DOUBLE_NODE;
    node->data->j_double = number;

    return node;
}

Sk_JsonNode*
Sk_JsonBoolNode_new(Sk_JsonBool boolean)
{
    Sk_JsonNode* node = Sk_JsonNode_default();

    if(node == NULL) {
        return NULL;
    }

    node->type            = SK_BOOL_NODE;
    node->data->j_boolean = boolean;

    return node;
}

Sk_JsonNode*
Sk_JsonNullNode_new(void)
{
    Sk_JsonNode* node = Sk_JsonNode_default();

    if(node == NULL) {
        return NULL;
    }

    node->type         = SK_NULL_NODE;
    node->data->j_null = NULL;

    return node;
}

void
Sk_JsonObjectNode_drop(Sk_JsonObject* object)
{
    if(object != NULL) {
        Sk_Vec_drop(&object->members, (FreeFn) Sk_JsonMember_drop);
    }
}

void
Sk_JsonArray_drop(Sk_JsonArray* array)
{
    if(array != NULL) {
        Sk_Vec_drop(&array->nodes, (FreeFn) Sk_JsonArray_drop);
    }
}

void
Sk_JsonMember_drop(Sk_JsonMember* member)
{
    if(member != NULL) {
        free(member->string);
        Sk_JsonNode_drop(member->value);
        member->string = NULL;
    }
}

void
Sk_JsonNode_drop(Sk_JsonNode* node)
{
    if(node != NULL) {
        switch(node->type) {
            case SK_OBJECT_NODE:
                Sk_JsonObject_drop(&node->data->j_object);
                break;
            case SK_ARRAY_NODE:
                Sk_JsonArray_drop(&node->data->j_array);
                break;
            case SK_STRING_NODE:
                free(node->data->j_string);
                break;
            default:
                break;
        }

        free(node);
    }
}
