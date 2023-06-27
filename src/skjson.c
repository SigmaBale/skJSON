#ifdef SK_DBUG
#include <assert.h>
#endif
#include "skerror.h"
#include "skjson.h"
#include "skparser.h"
#include "skutils.h"
#include <stdlib.h>

/* TODO: Implement public interface around this struct */

skJsonNode*
skJson_new(void* buff, size_t bufsize)
{
    skScanner*  scanner;
    skJsonNode* json;

    if(is_null(buff) || bufsize == 0) {
        return NULL;
    }

    scanner = skScanner_new(buff, bufsize);
    if(is_null(scanner)) {
        return NULL;
    }

    /* Construct the parse tree */
    json = skJsonNode_new(scanner, NULL);

    /* We are done scanning */
    free(scanner);

    if(is_null(json)) {
        return NULL;
    }

    return json;
}

unsigned int
skJson_type(skJsonNode* json)
{
    return (is_some(json)) ? json->type : -1;
}

skJsonNode*
skJson_parent(skJsonNode* json)
{
    return (is_some(json) && is_some(json->parent)) ? json->parent : NULL;
}

void*
skJson_value(skJsonNode* json)
{
    if(is_null(json)) {
        return NULL;
    }

    switch(json->type) {
        case SK_ERROR_NODE:
            return json->data.j_err;
        case SK_ARRAY_NODE:
            return json->data.j_array;
        case SK_INT_NODE:
            return &json->data.j_int;
        case SK_DOUBLE_NODE:
            return &json->data.j_double;
        case SK_STRING_NODE:
            return json->data.j_string;
        case SK_OBJECT_NODE:
            return json->data.j_object;
        case SK_BOOL_NODE:
            return &json->data.j_boolean;
        case SK_NULL_NODE:
            return NULL;
    }
}

bool
skJson_array_insert_string(skJsonNode* json, char* string)
{
    skJsonNode* string_node;
    skScanner   scanner;

    if(is_null(json) && json->type != SK_ARRAY_NODE) {
        return false;
    }

    // TODO:  continue
    // skVec_insert(json->data.j_array, string);

    return true;
}

void
skJson_drop(skJsonNode** json)
{
    if(is_some(json) && is_some(*json)) {
        skJsonNode_drop(*json);
        *json = NULL;
    }
}

void
skJson_drop_whole(skJsonNode** json)
{
    skJsonNode* current;

    if(is_some(json) && is_some(*json)) {

        for(current = (*json)->parent; current->parent != NULL; current = current->parent)
        {
            ;
        }

        skJsonNode_drop(current);
        *json = NULL;
    }
}
