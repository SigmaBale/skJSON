#ifdef SK_DBUG
#include <assert.h>
#endif
#include "skerror.h"
#include "skjson.h"
#include "skparser.h"
#include "skutils.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Marker macro for private functions */
#define PRIVATE(ret)                static ret
/* Check if the node is valid (not null) and matches the type 't' */
#define valid_with_type(node, t)    ((node) != NULL && (node)->type == (t))
/* Check if the node is 'valid_with_type' and has no parent */
#define can_be_inserted(node, type) (valid_with_type((node), (type)) && (node)->parent == NULL)

typedef struct _Serializer Serializer;

PRIVATE(bool) Serializer_serialize(Serializer* serializer, skJsonNode* json);
PRIVATE(bool) Serializer_serialize_number(Serializer* serializer, skJsonNode* json);
PRIVATE(bool) Serializer_serialize_string(Serializer* serializer, const char*);
PRIVATE(bool) Serializer_serialize_bool(Serializer* serializer, bool boolean);
PRIVATE(bool) Serializer_serialize_null(Serializer* serializer);
PRIVATE(bool) Serializer_serialize_array(Serializer* serializer, skVec* array);
PRIVATE(bool) Serializer_serialize_object(Serializer* serializer, skVec* table);
PRIVATE(void) drop_nonprim_elements(skJsonNode* json);
PRIVATE(bool) array_push_node_checked(skJsonNode* json, skJsonNode* node);
PRIVATE(int) cmp_members(skJsonNode* a, skJsonNode* b);
PRIVATE(int) cmp_members_descending(skJsonNode* a, skJsonNode* b);

struct _Serializer {
    unsigned char* buffer;
    size_t         length;
    size_t         offset;
    size_t         depth;
    bool           expand;
    bool           user_provided;
};

/* clang-format off */

/* Create a new Json Element from the given buffer 'buff' of parsable text
 * with length 'bufsize', return NULL in case 'buff' and 'bufsize' are NULL or 0, 
 * or if Json Element construction failed, otherwise return the root Json Element. */
PUBLIC(skJsonNode*)
skJson_parse(char* buff, size_t bufsize)
{
    skScanner*  scanner;
    skJsonNode* json;

    if(is_null(buff) || bufsize == 0) {
        return NULL;
    }

    scanner = skScanner_new(buff, bufsize);
    if(is_null(scanner)) {
        return NULL;
    } else {
        skScanner_next(scanner); /* Fetch first token */
    }

    /* Construct the parse tree */
    json = skJsonNode_parse(scanner, NULL);

    /* We are done scanning */
    free(scanner);

    if(is_null(json)) {
        return NULL;
    }

    return json;
}

PUBLIC(char*) skJson_error(skJson *json)
{
    if(valid_with_type(json, SK_ERROR_NODE)) {
        return json->data.j_err;
    } else {
        THROW_ERR(WrongNodeType);
        return NULL;
    }
}

/* Drop the 'json' freeing all of its child elements */
PUBLIC(void)
skJson_drop(skJsonNode** json)
{
    if(is_some(json) && is_some(*json)) {
        skJsonNode_drop(*json);
        *json = NULL;
    }
}

/* Drop the 'json' including all of its parents and their child elements */
PUBLIC(void)
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

/* Return the type of 'json', 'json' being the valid json element */
PUBLIC(int)
skJson_type(skJsonNode* json)
{
    return (is_some(json)) ? (int)json->type : -1;
}

/* Return the parent of 'json', 'json' being the valid Json Element */
PUBLIC(skJsonNode*)
skJson_parent(skJsonNode* json)
{
    return (is_some(json) && is_some(json->parent)) ? json->parent : NULL;
}

/* Return the value of the 'json', json being the valid Json Element */
PUBLIC(void*)
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
        case SK_STRINGLIT_NODE:
            return json->data.j_string;
        case SK_OBJECT_NODE:
            return json->data.j_object;
        case SK_BOOL_NODE:
            return &json->data.j_boolean;
        case SK_NULL_NODE:
            return NULL;
        default:
#ifdef SK_DBUG
            assert(false);
#endif
            return NULL;
    }
}

PRIVATE(void)
drop_nonprim_elements(skJsonNode* json) {
    switch(json->type) {
        case SK_STRING_NODE:
            free(json->data.j_string);
            break;
        case SK_ARRAY_NODE:
            skVec_drop(json->data.j_array, (FreeFn) skJsonNode_drop);
            break;
        case SK_OBJECT_NODE:
            skVec_drop(json->data.j_object, (FreeFn) skJsonNode_drop);
            break;
        default:
            break;
    }
}

PUBLIC(skJsonNode*)
skJson_transform_into_int(skJsonNode* json, long int n)
{
    if(is_null(json)) {
        THROW_ERR(WrongNodeType);
        return NULL;
    }

    drop_nonprim_elements(json);

    json->data.j_int = n;
    json->type = SK_INT_NODE;

    return json;
}

PUBLIC(skJsonNode*)
skJson_transform_into_double(skJsonNode* json, double n)
{
    if(is_null(json)) {
        THROW_ERR(WrongNodeType);
        return NULL;
    }

    drop_nonprim_elements(json);

    json->data.j_double = n;
    json->type = SK_DOUBLE_NODE;

    return json;
}


PUBLIC(skJsonNode*)
skJson_transform_into_bool(skJsonNode* json, bool boolean)
{
    if(is_null(json)) {
        THROW_ERR(WrongNodeType);
        return NULL;
    }

    drop_nonprim_elements(json);

    json->data.j_boolean = boolean;
    json->type = SK_BOOL_NODE;

    return json;
}

PUBLIC(skJsonNode*)
skJson_transform_into_stringlit(skJsonNode* json, const char* string_ref)
{
    skStrSlice slice;

    if(is_null(json)) {
        THROW_ERR(WrongNodeType);
        return NULL;
    }

    slice = skSlice_new(string_ref, strlen(string_ref) - 1);

    if(!skJsonString_isvalid(&slice)) {
        THROW_ERR(InvalidString);
        return NULL;
    }

    drop_nonprim_elements(json);

    json->data.j_string = discard_const(string_ref);
    json->type = SK_STRINGLIT_NODE;

    return json;
}

PUBLIC(skJsonNode*)
skJson_transform_into_string(skJsonNode* json, const char* string)
{
    skStrSlice slice;
    char* new_str;

    if(is_null(json)) {
        THROW_ERR(WrongNodeType);
        return NULL;
    }

    slice = skSlice_new(string, strlen(string) - 1);

    if(!skJsonString_isvalid(&slice)) {
        THROW_ERR(InvalidString);
        return NULL;
    }

    new_str = strdup_ansi(string);

    if(!is_null(new_str)) {
        return NULL;
    }

    drop_nonprim_elements(json);

    json->data.j_string = new_str;
    json->type = SK_STRING_NODE;

    return json;
}

PUBLIC(skJsonNode*)
skJson_transform_into_empty_array(skJsonNode* json)
{
    skVec* array;

    if(is_null(json)) {
        THROW_ERR(WrongNodeType);
        return NULL;
    }

    array = skVec_new(sizeof(skJsonNode));

    if(is_null(array)) {
        return NULL;
    }

    drop_nonprim_elements(json);

    json->data.j_array = array;
    json->type = SK_ARRAY_NODE;

    return json;
}

PUBLIC(skJsonNode*)
skJson_transform_into_empty_object(skJsonNode* json)
{
    skVec* table;

    if(is_null(json)) {
        THROW_ERR(WrongNodeType);
        return NULL;
    }

    if(is_null(table = skVec_new(sizeof(skJsonNode)))) {
        return NULL;
    }

    drop_nonprim_elements(json);

    json->data.j_object = table;
    json->type = SK_OBJECT_NODE;

    return json;
}

PUBLIC(skJsonNode*)
skJson_integer_new(long int n) {
    return IntNode_new(n, NULL);
}

PUBLIC(bool)
skJson_integer_set(skJsonNode* json, long int n)
{
    if(!valid_with_type(json, SK_INT_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    json->data.j_int = n;
    return true;
}

PUBLIC(skJsonNode*)
skJson_double_new(double n)
{
    return DoubleNode_new(n, NULL);
}

PUBLIC(bool)
skJson_double_set(skJsonNode* json, double n)
{
    if(!valid_with_type(json, SK_DOUBLE_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    json->data.j_double = n;
    return true;
}

PUBLIC(skJsonNode*)
skJson_bool_new(bool boolean)
{
    return BoolNode_new(boolean, NULL);
}

PUBLIC(bool)
skJson_bool_set(skJsonNode* json, bool boolean)
{
    if(!valid_with_type(json, SK_BOOL_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    json->data.j_boolean = boolean;
    return true;
}

PUBLIC(skJsonNode*)
skJson_null_new(void) {
    return RawNode_new(SK_NULL_NODE, NULL);
}

PUBLIC(skJsonNode*)
skJson_string_new(const char* string)
{
    return StringNode_new(discard_const(string), SK_STRING_NODE, NULL);
}

PUBLIC(bool)
skJson_string_set(skJsonNode* json, const char* string)
{
    char* new_str;
    skStrSlice slice;

    if(!valid_with_type(json, SK_STRING_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    slice = skSlice_new(string, strlen(string) - 1);

    if(!skJsonString_isvalid(&slice)) {
        THROW_ERR(InvalidString);
        return false;
    }

    if((new_str = strdup_ansi(string)) == NULL) {
        return false;
    }

    /* Drop old string and set the new one */
    free(json->data.j_string);
    json->data.j_string = new_str;

    return true;
}

PUBLIC(skJsonNode*)
skJson_stringlit_new(const char* string)
{
    skStrSlice  slice;

    slice = skSlice_new(string, strlen(string) - 1);

    if(!skJsonString_isvalid(&slice)) {
        THROW_ERR(InvalidString);
        return NULL;
    }

    return StringNode_new(discard_const(string), SK_STRINGLIT_NODE, NULL);
}

PUBLIC(bool)
skJson_stringlit_set(skJsonNode* json, const char* string)
{
    skStrSlice slice;

    if(!valid_with_type(json, SK_STRINGLIT_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    slice = skSlice_new(string, strlen(string) - 1);

    if(!skJsonString_isvalid(&slice)) {
        THROW_ERR(InvalidString);
        return false;
    }

    json->data.j_string = discard_const(string);
    return true;
}

/* Create an empty Json Array */
PUBLIC(skJsonNode*)
skJson_array_new(void)
{
    return ArrayNode_new(NULL);
}

PUBLIC(bool)
skJson_array_push_str(skJsonNode* json, const char* string)
{
    skJsonNode* string_node;
    skVec*      array;

    /* Check if node is array node */
    if(!valid_with_type(json, SK_ARRAY_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    /* Create a new string node */
    if(is_null(string_node = skJson_string_new(string))) {
        return false;
    }

    array = json->data.j_array;

    /* Push the node into the array */
    if(skVec_push(array, string_node)) {
        string_node->index  = skVec_len(array) - 1;
        string_node->parent = json;
        return true;
    }
#ifdef SK_DBUG
    assert(is_null(string_node->parent));
#endif
    skJsonNode_drop(string_node);
    return false;
}

PUBLIC(bool)
skJson_array_insert_str(skJsonNode* json, const char* string, size_t index)
{
    skJsonNode* string_node;

    if(!valid_with_type(json, SK_ARRAY_NODE) || is_null(string)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    if(is_null(string_node = skJson_string_new(string))) {
        return false;
    }

    if(skVec_insert(json->data.j_array, string_node, index)) {
        string_node->index  = index;
        string_node->parent = json;
        return true;
    }
#ifdef SK_DBUG
    assert(is_null(string_node->parent));
#endif
    skJsonNode_drop(string_node);
    return false;
}

PUBLIC(bool)
skJson_array_push_strlit(skJsonNode* json, const char* string)
{
    skJsonNode* string_node;
    skVec*      array;

    if(!valid_with_type(json, SK_ARRAY_NODE) || is_null(string)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    if(is_null(string_node = skJson_stringlit_new(string))) {
        return false;
    }

    array = json->data.j_array;

    if(skVec_push(array, string_node)) {
        string_node->index  = skVec_len(array) - 1;
        string_node->parent = json;
        return true;
    }
#ifdef SK_DBUG
    assert(is_null(string_node->parent));
#endif
    skJsonNode_drop(string_node);
    return false;
}

PUBLIC(bool)
skJson_array_insert_strlit(skJsonNode* json, const char* string, size_t index)
{
    skJsonNode* string_node;

    if(!valid_with_type(json, SK_ARRAY_NODE) || is_null(string)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    if(is_null(string_node = skJson_stringlit_new(string))) {
        return false;
    }

    if(!skVec_insert(json->data.j_array, string_node, index)) {
        string_node->index  = index;
        string_node->parent = json;
        return true;
    }
#ifdef SK_DBUG
    assert(is_null(string_node->parent));
#endif
    skJsonNode_drop(string_node);
    return false;
}

PUBLIC(bool)
skJson_array_push_int(skJsonNode* json, long int n)
{
    skJsonNode* int_node;
    skVec*      array;

    if(!valid_with_type(json, SK_ARRAY_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    if(is_null(int_node = IntNode_new(n, json))) {
        return false;
    }

    array = json->data.j_array;

    if(skVec_push(array, int_node)) {
        int_node->index = skVec_len(array) - 1;
        return true;
    }
#ifdef SK_DBUG
    assert(is_null(int_node->parent));
#endif
    skJsonNode_drop(int_node);
    return false;
}

PUBLIC(bool)
skJson_array_insert_int(skJsonNode* json, long int n, size_t index)
{
    skJsonNode* int_node;

    if(!valid_with_type(json, SK_ARRAY_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    if(is_null(int_node = IntNode_new(n, json))) {
        return false;
    }

    if(skVec_insert(json->data.j_array, int_node, index)) {
        int_node->index  = index;
        int_node->parent = json;
        return true;
    }
#ifdef SK_DBUG
    assert(is_null(int_node->parent));
#endif
    skJsonNode_drop(int_node);
    return false;
}

PUBLIC(bool)
skJson_array_push_double(skJsonNode* json, double n)
{
    skJsonNode* double_node;
    skVec*      array;

    if(!valid_with_type(json, SK_ARRAY_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    if(is_null(double_node = DoubleNode_new(n, json))) {
        return false;
    }

    array = json->data.j_array;

    if(skVec_push(array, double_node)) {
        double_node->index  = skVec_len(array) - 1;
        double_node->parent = json;
        return true;
    }
#ifdef SK_DBUG
    assert(is_null(double_node->parent));
#endif
    skJsonNode_drop(double_node);
    return false;
}

PUBLIC(bool)
skJson_array_insert_double(skJsonNode* json, double n, size_t index)
{
    skJsonNode* double_node;

    if(!valid_with_type(json, SK_ARRAY_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    if(is_null(double_node = DoubleNode_new(n, json))) {
        return false;
    }

    if(skVec_insert(json->data.j_array, double_node, index)) {
        double_node->index  = index;
        double_node->parent = json;
        return true;
    }
#ifdef SK_DBUG
    assert(is_null(double_node->parent));
#endif
    skJsonNode_drop(double_node);
    return false;
}

PUBLIC(bool)
skJson_array_push_bool(skJsonNode* json, bool boolean)
{
    skJsonNode* bool_node;
    skVec*      array;

    if(!valid_with_type(json, SK_ARRAY_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    if(is_null(bool_node = BoolNode_new(boolean, json))) {
        return false;
    }

    array = json->data.j_array;

    if(skVec_push(array, bool_node)) {
        bool_node->index  = skVec_len(array) - 1;
        bool_node->parent = json;
        return true;
    }
#ifdef SK_DBUG
    assert(is_null(bool_node->parent));
#endif
    skJsonNode_drop(bool_node);
    return false;
}

PUBLIC(bool)
skJson_array_insert_bool(skJsonNode* json, bool boolean, size_t index)
{
    skJsonNode* bool_node;

    if(!valid_with_type(json, SK_ARRAY_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    if(is_null(bool_node = BoolNode_new(boolean, json))) {
        return false;
    }

    if(skVec_push(json->data.j_array, bool_node)) {
        bool_node->index  = index;
        bool_node->parent = json;
        return true;
    }
#ifdef SK_DBUG
    assert(is_null(bool_node->parent));
#endif
    skJsonNode_drop(bool_node);
    return false;
}

PUBLIC(bool)
skJson_array_push_null(skJsonNode* json)
{
    skJsonNode* null_node;
    skVec*      array;

    if(!valid_with_type(json, SK_NULL_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    if(is_null(null_node = RawNode_new(SK_NULL_NODE, json))) {
        return false;
    }

    array = json->data.j_array;

    if(skVec_push(array, null_node)) {
        null_node->index = skVec_len(array) - 1;
        return true;
    }

    skJsonNode_drop(null_node);
    return false;
}

PUBLIC(bool)
skJson_array_insert_null(skJsonNode* json, size_t index)
{
    skJsonNode* null_node;

    if(!valid_with_type(json, SK_NULL_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    if(is_null(null_node = RawNode_new(SK_NULL_NODE, json))) {
        return false;
    }

    if(skVec_push(json->data.j_array, null_node)) {
        null_node->index = index;
        return true;
    }

    skJsonNode_drop(null_node);
    return false;
}

PUBLIC(bool)
skJson_array_push_element(skJsonNode* json, skJsonNode* element) {
    skVec* array;

    if(!valid_with_type(json, SK_ARRAY_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    array = json->data.j_array;

    if(skVec_push(array, element)) {
        element->parent = json;
        element->index = skVec_len(array) - 1;
        return true;
    }

    return false;
}

PUBLIC(bool)
skJson_array_insert_element(skJsonNode* json, skJsonNode* element, size_t index) {
    if(!valid_with_type(json, SK_ARRAY_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    if(skVec_insert(json->data.j_array, element, index)) {
        element->parent = json;
        element->index = index;
        return true;
    }

    return false;
}

PUBLIC(skJsonNode*)
skJson_array_from_strings(const char* const* strings, size_t count)
{
    skJsonNode* arr_node;

    if(is_null(arr_node = ArrayNode_new(NULL))) {
        return NULL;
    }

    for(; count--; strings++) {
        skJson_array_push_str(arr_node, discard_const(*strings));
    }

    return arr_node;
}

PUBLIC(skJsonNode*)
skJson_array_from_litstrings(const char* const* strings, size_t count)
{
    skJsonNode* arr_node;

    if(is_null(arr_node = ArrayNode_new(NULL))) {
        return NULL;
    }

    while(count--) {
        if(!skJson_array_push_strlit(arr_node, *strings++)) {
            skJsonNode_drop(arr_node);
            return NULL;
        }
    }

    return arr_node;
}

PUBLIC(skJsonNode*)
skJson_array_from_integers(const int* integers, size_t count)
{
    skJsonNode* arr_node;

    if(is_null(arr_node = ArrayNode_new(NULL))) {
        return NULL;
    }

    while(count--) {
        if(!skJson_array_push_int(arr_node, *integers)) {
            skJsonNode_drop(arr_node);
            return NULL;
        }
    }

    return arr_node;
}

PUBLIC(skJsonNode*)
skJson_array_from_doubles(const double* doubles, size_t count)
{
    skJsonNode* arr_node;

    if(is_null(arr_node = ArrayNode_new(NULL))) {
        return NULL;
    }

    while(count--) {
        if(!skJson_array_push_int(arr_node, *doubles++)) {
            skJsonNode_drop(arr_node);
            return NULL;
        }
    }

    return arr_node;
}

PUBLIC(skJsonNode*)
skJson_array_from_booleans(const bool* booleans, size_t count)
{
    skJsonNode* arr_node;

    if(is_null(arr_node = ArrayNode_new(NULL))) {
        return NULL;
    }

    while(count--) {
        if(!skJson_array_push_int(arr_node, *booleans++)) {
            skJsonNode_drop(arr_node);
            return NULL;
        }
    }

    return arr_node;
}

PUBLIC(skJsonNode*)
skJson_array_from_nulls(size_t count)
{
    skJsonNode* arr_node;

    if(is_null(arr_node = ArrayNode_new(NULL))) {
        return NULL;
    }

    while(count--) {
        if(!skJson_array_push_null(arr_node)) {
            skJsonNode_drop(arr_node);
            return NULL;
        }
    }

    return arr_node;
}

PUBLIC(skJsonNode*)
skJson_array_from_elements(const skJsonNode* const* elements, size_t count)
{
    skJsonNode* arr_node;

    if(is_null(arr_node = ArrayNode_new(NULL))) {
        return NULL;
    }

    while(count--) {
        if(is_null(*elements)
           || !array_push_node_checked(arr_node, discard_const(*elements++)))
        {
            free(arr_node);
            return NULL;
        }
    }

    return arr_node;
}

PRIVATE(bool)
array_push_node_checked(skJsonNode* json, skJsonNode* node)
{
    if(skVec_push(json->data.j_array, node)) {
        node->parent = json;
        node->index  = skVec_len(json->data.j_array) - 1;
        return true;
    }

    return false;
}

PUBLIC(skJsonNode*)
skjson_array_pop(skJsonNode* json)
{
    skJsonNode* node;

    if(!valid_with_type(json, SK_ARRAY_NODE)) {
        THROW_ERR(WrongNodeType);
        return NULL;
    }

    node = skVec_pop(json->data.j_array);

    if(is_some(node)) {
        node->parent = NULL;
    }

    return node;
}

PUBLIC(bool)
skjson_array_remove(skJsonNode* json, size_t index)
{
    if(!valid_with_type(json, SK_ARRAY_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    return skVec_remove(json->data.j_array, index, (FreeFn) skJsonNode_drop);
}

PUBLIC(size_t)
skJson_array_len(skJsonNode* json)
{
    if(!valid_with_type(json, SK_ARRAY_NODE)) {
        THROW_ERR(WrongNodeType);
        return 0;
    }

    return skVec_len(json->data.j_array);
}

PUBLIC(skJsonNode*)
skJson_array_front(skJsonNode* json)
{
    if(!valid_with_type(json, SK_ARRAY_NODE)) {
        THROW_ERR(WrongNodeType);
        return NULL;
    }

    return skVec_front(json->data.j_array);
}

PUBLIC(skJsonNode*)
skJson_array_back(skJsonNode* json)
{
    if(!valid_with_type(json, SK_ARRAY_NODE)) {
        THROW_ERR(WrongNodeType);
        return NULL;
    }

    return skVec_back(json->data.j_array);
}

PUBLIC(skJsonNode*)
skJson_array_index(skJsonNode* json, size_t index)
{
    if(!valid_with_type(json, SK_ARRAY_NODE)) {
        THROW_ERR(WrongNodeType);
        return NULL;
    }

    return skVec_index(json->data.j_array, index);
}

PUBLIC(void)
skJson_array_clear(skJson* json)
{
    if(!valid_with_type(json, SK_ARRAY_NODE)) {
        THROW_ERR(WrongNodeType);
        return;
    }

    skVec_clear(json->data.j_array, (FreeFn) skJsonNode_drop);
}

PUBLIC(skJsonNode*)
skJson_object_new(void) {
    return ObjectNode_new(NULL);
}

PUBLIC(bool)
skJson_object_sort_ascending(skJsonNode* json)
{
    if(!valid_with_type(json, SK_OBJECT_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    return skVec_sort(json->data.j_object, (CmpFn) cmp_members);
}

PRIVATE(int)
cmp_members(skJsonNode* a, skJsonNode* b)
{
    return strcmp(a->data.j_member.key, b->data.j_member.key);
}

PUBLIC(bool)
skJson_object_sort_descending(skJsonNode* json)
{
    if(!valid_with_type(json, SK_OBJECT_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    return skVec_sort(json->data.j_object, (CmpFn) cmp_members_descending);
}

PRIVATE(int)
cmp_members_descending(skJsonNode* a, skJsonNode* b)
{
    int cmp;

    if((cmp = cmp_members(a, b)) < 0) {
        return 1;
    } else if(cmp > 0) {
        return -1;
    }

    return 0;
}

PUBLIC(bool)
skJson_object_insert_element(skJsonNode* json, const char* key, const skJsonNode* elementp, size_t index)
{
    skJsonNode* member_node;

    if(!valid_with_type(json, SK_OBJECT_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    if(is_null(member_node = MemberNode_new(key, elementp, json))) {
        return false;
    }

    if(!skVec_insert(json->data.j_object, member_node, index)) {
        free(member_node);
        return false;
    }

    return true;
}

PUBLIC(bool)
skJson_object_remove_element(skJsonNode* json, size_t index)
{
    if(!valid_with_type(json, SK_OBJECT_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    return skVec_remove(json->data.j_object, index, (FreeFn) skJsonNode_drop);
}

PUBLIC(bool)
skJson_object_remove_element_by_key(skJsonNode* json, const char* key, bool sorted, bool descending)
{
    skJsonNode dummynode;

    if(!valid_with_type(json, SK_OBJECT_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    dummynode.data.j_member.key = discard_const(key);

    return skVec_remove_by_key(
            json->data.j_object,
            &dummynode,
            (descending) ? (CmpFn) cmp_members_descending : (CmpFn) cmp_members,
            (FreeFn) skJsonNode_drop,
            sorted);
}

PUBLIC(skJsonNode*)
skJson_object_get_element(const skJsonNode* json, size_t index)
{
    if(!valid_with_type(json, SK_OBJECT_NODE)) {
        THROW_ERR(WrongNodeType);
        return NULL;
    }

    return skVec_index(json->data.j_object, index);
}

PUBLIC(skJsonNode*)
skJson_object_get_element_by_key(
        const skJsonNode* json,
        const char* key,
        bool sorted,
        bool descending)
{
    skJsonNode dummynode;

    if(!valid_with_type(json, SK_OBJECT_NODE)) {
        THROW_ERR(WrongNodeType);
        return NULL;
    }

    dummynode.data.j_member.key = discard_const(key);

    return skVec_get_by_key(
            json->data.j_object,
            &dummynode,
            (descending) ? (CmpFn) cmp_members_descending : (CmpFn) cmp_members,
            sorted);
}

PUBLIC(size_t)
skJson_object_len(const skJsonNode* json)
{
    if(!valid_with_type(json, SK_OBJECT_NODE)) {
        THROW_ERR(WrongNodeType);
        return 0;
    }

    return skVec_len(json->data.j_object);
}

PUBLIC(bool)
skJson_object_contains(const skJsonNode* json, const char* key, bool sorted, bool descending)
{
    skJsonNode dummynode;

    if(!valid_with_type(json, SK_OBJECT_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    dummynode.data.j_member.key = discard_const(key);

    return skVec_contains(json->data.j_object,
            &dummynode,
            (descending) ? (CmpFn) cmp_members_descending : (CmpFn) cmp_members,
            sorted);
}

PUBLIC(void)
skJson_object_clear(skJson* json)
{
    if(!valid_with_type(json, SK_OBJECT_NODE)) {
        THROW_ERR(WrongNodeType);
        return;
    }

    skVec_clear(json->data.j_object, (FreeFn) skJsonNode_drop);
}

PRIVATE(Serializer)
Serializer_new(size_t bufsize, bool expand)
{
    Serializer pbuf;
    unsigned char* buffer;

    memset(&pbuf, 0, sizeof(Serializer));

    if(bufsize == 0) {
        bufsize = BUFSIZ;
    }

    if(is_null(buffer = malloc(bufsize))) {
        THROW_ERR(OutOfMemory);
        return pbuf;
    }

    pbuf.length = bufsize;
    pbuf.buffer = buffer;
    pbuf.expand = expand;

    return pbuf;
}

PRIVATE(Serializer)
Serializer_from(unsigned char* buffer, size_t bufsize, bool expand)
{
    Serializer pbuf;

    memset(&pbuf, 0, sizeof(Serializer));
#ifdef SK_DBUG
    if(is_null(buffer) || bufsize == 0) {
        return pbuf;
    }
#endif
    pbuf.length = bufsize;
    pbuf.buffer = buffer;
    pbuf.expand = expand;
    pbuf.user_provided = true;

    return pbuf;
}

PRIVATE(void)
Serializer_drop(Serializer* serializer)
{
#ifdef SK_DBUG
    assert(is_some(serializer));
    assert(is_some(serializer->buffer));
#endif
    free(serializer->buffer);
    memset(serializer, 0, sizeof(Serializer));
}

PRIVATE(unsigned char*)
Serializer_buffer_ensure(Serializer* serializer, size_t needed)
{
    unsigned char* newbuf;
    size_t newsize;

#ifdef SK_DBUG
    assert(is_some(serializer));
    assert(is_some(serializer->buffer));
    assert(!(serializer->length > 0 && serializer->offset >= serializer->length));
#endif

    if(needed > INT_MAX) {
        THROW_ERR(AllocationTooLarge);
        if(!serializer->user_provided) {
            Serializer_drop(serializer);
        }
        return NULL;
    }

    needed += serializer->offset + 1; /* 1 for null terminator */

    /* If we got enough space return current position */
    if(needed <= serializer->length) {
        return serializer->buffer + serializer->offset;
    }

    /* Check if we are allowed to expand */
    if(!serializer->expand) {
        if(!serializer->user_provided) {
            Serializer_drop(serializer);
        }
        return NULL;
    }

    /* Else try expand the buffer */
    if(needed > (INT_MAX/2)) {
        if(needed <= INT_MAX) {
            newsize = INT_MAX;
        } else {
            THROW_ERR(AllocationTooLarge);
            if(!serializer->user_provided) {
                Serializer_drop(serializer);
            }
            return NULL;
        }
    } else {
        newsize = needed * 2;
    }

    /* Try reallocate the buffer to 'newsize' */
    newbuf = realloc(serializer->buffer, newsize);

    if(is_null(newbuf)) {
        THROW_ERR(OutOfMemory);
        if(!serializer->user_provided) {
            Serializer_drop(serializer);
        }
        return NULL;
    }

    serializer->buffer = newbuf;
    serializer->length = newsize;

    return newbuf + serializer->offset;
}

PRIVATE(void)
Serializer_offset_update(Serializer* serializer)
{
#ifdef SK_DBUG
    assert(is_some(serializer));
    assert(is_some(serializer->buffer));
#endif
    printf("Before updating offset -> %ld\n", serializer->offset);
    serializer->offset += strlen((char*)serializer->buffer + serializer->offset);
    printf("After updating offset -> %ld\n", serializer->offset);
}

PUBLIC(unsigned char*)
skJson_serialize_with_buffer(skJsonNode* json, unsigned char* buffer, size_t size, bool expand)
{
    Serializer serializer;

    if(is_null(json) || is_null(buffer) || size == 0) {
        return NULL;
    }

    if(json->type == SK_ERROR_NODE) {
        THROW_ERR(WrongNodeType);
        return NULL;
    }

    serializer = Serializer_from(buffer, size, expand);

#ifdef SK_DBUG
    assert(is_some(serializer.buffer));
#endif

    if(!Serializer_serialize(&serializer, json)) {
        return NULL;
    }

    return serializer.buffer;
}

PUBLIC(unsigned char*)
skJson_serialize_with_bufsize(skJsonNode* json, size_t size, bool expand)
{
    Serializer serializer;

    if(is_null(json) || size == 0) {
        return NULL;
    }

    if(json->type == SK_ERROR_NODE) {
        THROW_ERR(WrongNodeType);
        return NULL;
    }

    if(is_null((serializer = Serializer_new(size, expand)).buffer)) {
        return NULL;
    }

    if(!Serializer_serialize(&serializer, json)) {
        return NULL;
    }

    return serializer.buffer;
}

PUBLIC(unsigned char*)
skJson_serialize(skJsonNode* json)
{
    Serializer serializer;

    if(is_null(json)) {
        return NULL;
    }

    if(json->type == SK_ERROR_NODE) {
        THROW_ERR(WrongNodeType);
        return NULL;
    }

    if(is_null((serializer = Serializer_new(0, true)).buffer)) {
        return NULL;
    }
    
    if(!Serializer_serialize(&serializer, json)) {
        return NULL;
    }

    printf("Total len in bytes -> %ld\n", strlen((char*)serializer.buffer));
    return serializer.buffer;
}

PRIVATE(bool)
Serializer_serialize(Serializer* serializer, skJsonNode* json)
{
#ifdef SK_DBUG
    assert(is_some(json));
    assert(is_some(serializer));
    assert(is_some(serializer->buffer));
#endif
    switch(json->type) {
        case SK_STRING_NODE:
        case SK_STRINGLIT_NODE:
            return Serializer_serialize_string(serializer, json->data.j_string);
        case SK_INT_NODE:
        case SK_DOUBLE_NODE:
            return Serializer_serialize_number(serializer, json);
        case SK_BOOL_NODE:
            return Serializer_serialize_bool(serializer, json->data.j_boolean);
        case SK_NULL_NODE:
            return Serializer_serialize_null(serializer);
        case SK_ARRAY_NODE:
            return Serializer_serialize_array(serializer, json->data.j_array);
        case SK_OBJECT_NODE:
            printf("Serializing object\n");
            return Serializer_serialize_object(serializer, json->data.j_object);
        case SK_ERROR_NODE:
        default:
            THROW_ERR(SerializerInvalidJson);
            return false;
    }
}

PRIVATE(bool)
Serializer_serialize_number(Serializer *serializer, skJsonNode* json)
{
    char buff[30];
    unsigned char* out;
    long int len;

#ifdef SK_DBUG
    assert(is_some(serializer));
    assert(is_some(serializer->buffer));
    assert(is_some(json));
#endif

    if(json->type == SK_DOUBLE_NODE) {
        len = sprintf(buff, "%fl", json->data.j_double);
    } else {
        len = sprintf(buff, "%ld", json->data.j_int);
    }

    if(len < 0 || len >= 30) {
        THROW_ERR(SerializerNumberError);
        return false;
    }

    out = Serializer_buffer_ensure(serializer, len + sizeof("")); /* len + '\0' */

    if(is_null(out)) {
        return false;
    }

    strcat((char*)out, buff);
    Serializer_offset_update(serializer);

    return true;
}

PRIVATE(bool)
Serializer_serialize_string(Serializer *serializer, const char* str)
{
    unsigned char* out;
    size_t len;

#ifdef SK_DBUG
    assert(is_some(serializer));
    assert(is_some(serializer->buffer));
#endif

    len = strlen(str) + sizeof("\"\"");
    out = Serializer_buffer_ensure(serializer, 1 + len + sizeof("\""));

    if(is_null(out)) {
        return false;
    }

    out[0] = '\"';
    memcpy(out + 1, str, len);
    out[len + 1] = '\"';
    out[len + 2] = '\0';

    printf("Offset is %ld\n", serializer->offset);
    Serializer_offset_update(serializer);

    return true;
}

PRIVATE(bool)
Serializer_serialize_bool(Serializer *serializer, bool boolean)
{
    unsigned char* out;

#ifdef SK_DBUG
    assert(is_some(serializer));
    assert(is_some(serializer->buffer));
#endif

    if(boolean) {
        if(is_null(out = Serializer_buffer_ensure(serializer, 5))) {
            return false;
        }
        strcpy((char*)out, "true");
    } else {
        if(is_null(out = Serializer_buffer_ensure(serializer, 6))) {
            return false;
        }
        strcpy((char*)out, "false");
    }

    Serializer_offset_update(serializer);
    printf("%s\n", serializer->buffer);
#ifdef SK_DBUG
    assert(strcmp((char*)serializer->buffer, "{\"verifiable_password_authentication\": false") == 0);
#endif
    return true;
}

PRIVATE(bool)
Serializer_serialize_null(Serializer *serializer)
{
    unsigned char* out;

#ifdef SK_DBUG
    assert(is_some(serializer));
    assert(is_some(serializer->buffer));
#endif

    if(is_null(out = Serializer_buffer_ensure(serializer, 5))) {
        return false;
    }
    strcpy((char*)out, "null");
    Serializer_offset_update(serializer);
    return true;
}

PRIVATE(bool)
Serializer_serialize_array(Serializer *serializer, skVec* vec)
{
    unsigned char* out;
    size_t len, i;
    skJsonNode* current;

#ifdef SK_DBUG
    assert(is_some(serializer));
    assert(is_some(serializer->buffer));
#endif

    if(is_null(out = Serializer_buffer_ensure(serializer, 1))) {
        return false;
    }

    out[0] = '[';
    serializer->offset++;
    serializer->depth++;

    len = skVec_len(vec);

    for(i = 0; i < len; i++) {
        current = skVec_index(vec, i);
#ifdef SK_DBUG
        assert(is_some(current));
#endif
        if(!Serializer_serialize(serializer, current)) {
            return false;
        }

        if(i + 1 < len) {
            if(is_null(out = Serializer_buffer_ensure(serializer, 2))) {
                return false;
            }
            *out++ = ',';
            *out = '\0';
            serializer->offset += 2;
        }
    }

    if(is_null(out = Serializer_buffer_ensure(serializer, 2))) {
        return false;
    }

    *out++ = ']';
    *out = '\0';
    serializer->depth--;
    serializer->offset += 2;

    return true;
}

PRIVATE(bool)
Serializer_serialize_object(Serializer* serializer, skVec* table)
{
    unsigned char* out;
    skJsonNode* member_node;
    size_t len, i;

#ifdef SK_DBUG
    assert(is_some(serializer));
    assert(is_some(serializer->buffer));
    assert(is_some(table));
#endif

    if(is_null(out = Serializer_buffer_ensure(serializer, 1))) {
        return false;
    }

    out[0] = '{';
    serializer->offset++;
    serializer->depth++;

    len = skVec_len(table);
    printf("table len is -> %ld\n", len);
    for(i = 0; i < len; i++) {
        member_node = skVec_index(table, i);
#ifdef SK_DBUG
        assert(is_some(member_node));
#endif
        if(!Serializer_serialize_string(serializer, member_node->data.j_member.key)) {
            return false;
        }

        if(is_null(out = Serializer_buffer_ensure(serializer, 2))) {
            return false;
        }

        *out = ':';
        serializer->offset+=2;

        if(!Serializer_serialize(serializer, member_node->data.j_member.value)) {
            return false;
        }

        if(is_null(out = Serializer_buffer_ensure(serializer, 2))) {
            return false;
        }

        if(i + 1 != len) {
            *out++ = ',';
            *out = '\0';
            serializer->offset += 2;
        }
    }

    *out++ = '}';
    *out = '\0';
    serializer->offset += 2;
    serializer->depth--;

    return true;
}
