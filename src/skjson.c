#ifdef SK_DBUG
#include <assert.h>
#endif
#include "skerror.h"
#include "skjson.h"
#include "skparser.h"
#include "skutils.h"
#include <stdlib.h>
#include <string.h>

/* Marker macro for private functions */
#define PRIVATE(ret)                static ret
/* Check if the node is valid (not null) and matches the type 't' */
#define valid_with_type(node, t)    ((node) != NULL && (node)->type == (t))
/* Check if the node is 'valid_with_type' and has no parent */
#define can_be_inserted(node, type) (valid_with_type((node), (type)) && (node)->parent == NULL)

PRIVATE(skJsonNode*) _skJsonRawNode_new(skNodeType type, skJsonNode* parent);
PRIVATE(skJsonNode*) _skJsonRawIntNode_new(long int n, skJsonNode* parent);
PRIVATE(skJsonNode*) _skJsonRawDoubleNode_new(double n, skJsonNode* parent);
PRIVATE(skJsonNode*) _skJsonRawBoolNode_new(bool b, skJsonNode* parent);
PRIVATE(skJsonNode*) _skJsonRawStringNode_new(const char* string, skNodeType type);
PRIVATE(skJsonNode*) _skJsonRawObjectNode_new(skJsonNode* parent);
PRIVATE(skJsonNode*) _skJsonRawArrayNode_new(skJsonNode* parent);
PRIVATE(void) _skJson_drop_nonprim(skJsonNode* json);
PRIVATE(bool) _skJson_array_push_node_checked(skJsonNode* json, skJsonNode* node);

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
PUBLIC(unsigned int)
skJson_type(skJsonNode* json)
{
    return (is_some(json)) ? json->type : -1;
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
    }
}

PRIVATE(void)
_skJson_drop_nonprim(skJsonNode* json) {
    switch(json->type) {
        case SK_STRING_NODE:
            free(json->data.j_string);
            break;
        case SK_ARRAY_NODE:
            skVec_drop(json->data.j_array, (FreeFn) skJsonNode_drop);
            break;
        case SK_OBJECT_NODE:
            skHashTable_drop(json->data.j_object);
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

    _skJson_drop_nonprim(json);

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

    _skJson_drop_nonprim(json);

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

    _skJson_drop_nonprim(json);

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

    _skJson_drop_nonprim(json);

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

    _skJson_drop_nonprim(json);

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

    _skJson_drop_nonprim(json);

    json->data.j_array = array;
    json->type = SK_ARRAY_NODE;

    return json;
}

PUBLIC(skJsonNode*)
skJson_transform_into_empty_object(skJsonNode* json)
{
    skHashTable* object;

    if(is_null(json)) {
        THROW_ERR(WrongNodeType);
        return NULL;
    }

    object = skHashTable_new(
            (HashFn) NULL,
            (CmpKeyFn) strcmp,
            (FreeKeyFn) free,
            (FreeValueFn) skJsonNode_drop);

    if(is_null(object)) {
        return NULL;
    }

    _skJson_drop_nonprim(json);

    json->data.j_object = object;
    json->type = SK_ARRAY_NODE;

    return json;
}

PUBLIC(skJsonNode*)
skJson_integer_new(long int n) {
    return _skJsonRawIntNode_new(n, NULL);
}

PRIVATE(skJsonNode*)
_skJsonRawIntNode_new(long int n, skJsonNode* parent)
{
    skJsonNode* int_node;

    int_node = _skJsonRawNode_new(SK_INT_NODE, parent);
    if(is_null(int_node)) {
        return NULL;
    }

    int_node->data.j_int = n;
    return int_node;
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
    return _skJsonRawDoubleNode_new(n, NULL);
}

PRIVATE(skJsonNode*)
_skJsonRawDoubleNode_new(double n, skJsonNode* parent)
{
    skJsonNode* double_node;

    double_node = _skJsonRawNode_new(SK_DOUBLE_NODE, parent);
    if(is_null(double_node)) {
        return NULL;
    }

    double_node->data.j_double = n;
    return double_node;
}

PUBLIC(bool)
skJson_double_set(skJsonNode* json, double n)
{
    if(!valid_with_type(json, SK_DOUBLE_NODE)) {
        THROW_ERR(WrongNodeType);
        return NULL;
    }

    json->data.j_double = n;
    return true;
}

PUBLIC(skJsonNode*)
skJson_bool_new(bool boolean)
{
    return _skJsonRawBoolNode_new(boolean, NULL);
}

PRIVATE(skJsonNode*)
_skJsonRawBoolNode_new(bool b, skJsonNode* parent)
{
    skJsonNode* bool_node;

    bool_node = _skJsonRawNode_new(SK_BOOL_NODE, parent);
    if(is_null(bool_node)) {
        return NULL;
    }

    bool_node->data.j_boolean = b;
    return bool_node;
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
    return _skJsonRawNode_new(SK_NULL_NODE, NULL);
}

PRIVATE(skJsonNode*)
_skJsonRawNode_new(skNodeType type, skJsonNode* parent)
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

/* Try creating a valid Json String from 'string' that holds the newly allocated
 * duplicated string */
PUBLIC(skJsonNode*)
skJson_string_new(const char* string)
{
    return _skJsonRawStringNode_new(string, SK_STRING_NODE);
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

/* Try creating a valid Json String from 'string' that holds the reference to it */
PUBLIC(skJsonNode*)
skJson_stringlit_new(const char* string)
{
    return _skJsonRawStringNode_new(string, SK_STRINGLIT_NODE);
}

/* Check validity and construct the valid Json string from 'string' of type 'type' */
PRIVATE(skJsonNode*)
_skJsonRawStringNode_new(const char* string, skNodeType type)
{
    skJsonNode* string_node;
    skStrSlice  slice;
    char*       dup_str;

    /* Take range up to the null-terminator */
    slice = skSlice_new(string, strlen(string) - 1);

    /* Check if string is valid json string */
    if(!skJsonString_isvalid(&slice)) {
        THROW_ERR(InvalidString);
        return NULL;
    }

    /* If type is not a reference string then duplicate it */
    if(type == SK_STRING_NODE && is_null(dup_str = strdup_ansi(string))) {
        return NULL;
    } else {
        dup_str = discard_const(string);
    }

    /* Create a node */
    string_node = _skJsonRawNode_new(type, NULL);
    if(is_null(string_node)) {
        return NULL;
    }

    /* Store the 'dup_str' into the node */
    string_node->data.j_string = dup_str;
    return string_node;
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
    return _skJsonRawArrayNode_new(NULL);
}

PRIVATE(skJsonNode*)
_skJsonRawArrayNode_new(skJsonNode* parent)
{
    skJsonNode* array_node;
    skVec* array;

    if(is_null(array_node = _skJsonRawNode_new(SK_ARRAY_NODE, parent))) {
        return NULL;
    }

    array = array_node->data.j_array;

    if(is_null(array = skVec_new(sizeof(skJsonNode)))) {
        free(array_node);
        return NULL;
    }

    return array_node;
}

/* Push the 'string' into the 'json', 'string' being valid Json String and
 * 'json' being the valid Json Array */
PUBLIC(bool)
skJson_array_push_str(skJsonNode* json, const char* string)
{
    skJsonNode* string_node;
    skStrSlice  slice;
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

/* Insert the 'string' into the 'json' at the index 'index', 'string' being
 * the valid Json String and 'json' being the valid Json Array */
PUBLIC(bool)
skJson_array_insert_str(skJsonNode* json, const char* string, size_t index)
{
    skJsonNode* string_node;
    skStrSlice  slice;

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

/* Push the string literal 'string' into the 'json', 'string' being the 
 * valid Json String and 'json' being the valid Json Array */
PUBLIC(bool)
skJson_array_push_strlit(skJsonNode* json, const char* string)
{
    skJsonNode* string_node;
    skStrSlice  slice;
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

/* Insert the string literal 'string' into the 'json' at the index 'index',
 * 'string' being the valid Json String and 'json' being the valid Json Array */
PUBLIC(bool)
skJson_array_insert_strlit(skJsonNode* json, const char* string, size_t index)
{
    skJsonNode* string_node;
    skStrSlice  slice;

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

/* Push the integer 'n' into the 'json', 'json' being the valid Json Array */
PUBLIC(bool)
skJson_array_push_int(skJsonNode* json, long int n)
{
    skJsonNode* int_node;
    skVec*      array;

    if(!valid_with_type(json, SK_ARRAY_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    if(is_null(int_node = _skJsonRawIntNode_new(n, json))) {
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

/* Insert the integer 'n' into 'json' at the index 'index', 'json' being
 * valid Json Array. */
PUBLIC(bool)
skJson_array_insert_int(skJsonNode* json, long int n, size_t index)
{
    skJsonNode* int_node;

    if(!valid_with_type(json, SK_ARRAY_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    if(is_null(int_node = _skJsonRawIntNode_new(n, json))) {
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

/* Insert the double 'n' into the 'json', 'json' being valid Json Array */
PUBLIC(bool)
skJson_array_push_double(skJsonNode* json, double n)
{
    skJsonNode* double_node;
    skVec*      array;

    if(!valid_with_type(json, SK_ARRAY_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    if(is_null(double_node = _skJsonRawDoubleNode_new(n, json))) {
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

/* Insert the double 'n' into the 'json' at the index 'index', 'json' being valid
 * Json Array */
PUBLIC(bool)
skJson_array_insert_double(skJsonNode* json, double n, size_t index)
{
    skJsonNode* double_node;

    if(!valid_with_type(json, SK_ARRAY_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    if(is_null(double_node = _skJsonRawDoubleNode_new(n, json))) {
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

/* Push the 'boolean' into the 'json', 'json' being valid Json Array. */
PUBLIC(bool)
skJson_array_push_bool(skJsonNode* json, bool boolean)
{
    skJsonNode* bool_node;
    skVec*      array;

    if(!valid_with_type(json, SK_ARRAY_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    if(is_null(bool_node = _skJsonRawBoolNode_new(boolean, json))) {
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

/* Insert the 'boolean' at the index 'index' into the 'json', 'json' being valid
 * Json Array. */
PUBLIC(bool)
skJson_array_insert_bool(skJsonNode* json, bool boolean, size_t index)
{
    skJsonNode* bool_node;

    if(!valid_with_type(json, SK_ARRAY_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    if(is_null(bool_node = _skJsonRawBoolNode_new(boolean, json))) {
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

/* Push the NULL into the 'json', json being the valid Json Array */
PUBLIC(bool)
skJson_array_push_null(skJsonNode* json)
{
    skJsonNode* null_node;
    skVec*      array;

    if(!valid_with_type(json, SK_NULL_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    if(is_null(null_node = _skJsonRawNode_new(SK_NULL_NODE, json))) {
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

/* Insert NULL at the index 'index' into the 'json', 'json' being the valid Json Array */
PUBLIC(bool)
skJson_array_insert_null(skJsonNode* json, size_t index)
{
    skJsonNode* null_node;

    if(!valid_with_type(json, SK_NULL_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    if(is_null(null_node = _skJsonRawNode_new(SK_NULL_NODE, json))) {
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



/* Create Json Array of Json strings */
PUBLIC(skJsonNode*)
skJson_array_from_strings(const char* const* strings, size_t count)
{
    skJsonNode* arr_node;

    if(is_null(arr_node = _skJsonRawArrayNode_new(NULL))) {
        return NULL;
    }

    for(; count--; strings++) {
        skJson_array_push_str(arr_node, discard_const(*strings));
    }

    return arr_node;
}

/* Create Json Array of references to Json strings */
PUBLIC(skJsonNode*)
skJson_array_from_litstrings(const char* const* strings, size_t count)
{
    skJsonNode* arr_node;

    if(is_null(arr_node = _skJsonRawArrayNode_new(NULL))) {
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

/* Create Json Array of Json integers */
PUBLIC(skJsonNode*)
skJson_array_from_integers(const int* integers, size_t count)
{
    skJsonNode* arr_node;

    if(is_null(arr_node = _skJsonRawArrayNode_new(NULL))) {
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

/* Create Json Array of Json doubles */
PUBLIC(skJsonNode*)
skJson_array_from_doubles(const double* doubles, size_t count)
{
    skJsonNode* arr_node;

    if(is_null(arr_node = _skJsonRawArrayNode_new(NULL))) {
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

/* Create Json Array of Json Booleans */
PUBLIC(skJsonNode*)
skJson_array_from_booleans(const bool* booleans, size_t count)
{
    skJsonNode* arr_node;

    if(is_null(arr_node = _skJsonRawArrayNode_new(NULL))) {
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

/* Create Json Array of NULL values */
PUBLIC(skJsonNode*)
skJson_array_from_nulls(size_t count)
{
    skJsonNode* arr_node;

    if(is_null(arr_node = _skJsonRawArrayNode_new(NULL))) {
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

    if(is_null(arr_node = _skJsonRawArrayNode_new(NULL))) {
        return NULL;
    }

    while(count--) {
        if(is_null(*elements)
           || !_skJson_array_push_node_checked(arr_node, discard_const(*elements++)))
        {
            free(arr_node);
            return NULL;
        }
    }

    return arr_node;
}

PRIVATE(bool)
_skJson_array_push_node_checked(skJsonNode* json, skJsonNode* node)
{
    if(skVec_push(json->data.j_array, node)) {
        node->parent = json;
        node->index  = skVec_len(json->data.j_array) - 1;
        return true;
    }

    return false;
}

/* Pop the element from the Json Array copying and returning it */
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

/* Remove the element from 'json' at index 'index' freeing its allocation and
 * its sub elements */
PUBLIC(bool)
skjson_array_remove(skJsonNode* json, size_t index)
{
    if(!valid_with_type(json, SK_ARRAY_NODE)) {
        THROW_ERR(WrongNodeType);
        return NULL;
    }

    return skVec_remove(json->data.j_array, index, (FreeFn) skJsonNode_drop);
}

/* Return the amount of elements stored in Json Array */
PUBLIC(size_t)
skJson_array_len(skJsonNode* json)
{
    if(!valid_with_type(json, SK_ARRAY_NODE)) {
        THROW_ERR(WrongNodeType);
        return NULL;
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

PUBLIC(skJsonNode*)
skJson_object_new(void) {
    return _skJsonRawObjectNode_new(NULL);
}

PRIVATE(skJsonNode*)
_skJsonRawObjectNode_new(skJsonNode* parent)
{
    skJsonNode* object_node;

    object_node = _skJsonRawNode_new(SK_OBJECT_NODE, parent);

    if(is_null(object_node)) {
        return NULL;
    }

    object_node->data.j_object = skHashTable_new(
        (HashFn) NULL,
        (CmpKeyFn) strcmp,
        (FreeKeyFn) free,
        (FreeValueFn) skJsonNode_drop);

    if(is_null(object_node->data.j_array)) {
        return NULL;
    }

    return object_node;
}

PUBLIC(bool)
skJson_object_insert_element(skJsonNode* json, char* key, skJsonNode* element)
{
    if(!valid_with_type(json, SK_OBJECT_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    return skHashTable_insert(json->data.j_object, key, element);
}

PUBLIC(bool)
skJson_object_remove_element(skJsonNode* json, char* key)
{
    if(!valid_with_type(json, SK_OBJECT_NODE)) {
        THROW_ERR(WrongNodeType);
        return false;
    }

    return skHashTable_remove(json->data.j_object, key);
}

PUBLIC(skJsonNode*)
skJson_object_get_element(skJsonNode* json, char* key)
{
    if(!valid_with_type(json, SK_OBJECT_NODE)) {
        THROW_ERR(WrongNodeType);
        return NULL;
    }

    return skHashTable_get(json->data.j_object, key);
}

PUBLIC(size_t)
skJson_object_len(skJsonNode* json)
{
    if(!valid_with_type(json, SK_OBJECT_NODE)) {
        THROW_ERR(WrongNodeType);
        return NULL;
    }

    return skHashTable_len(json->data.j_object);
}

PUBLIC(bool)
skJson_object_contains(const skJsonNode* json, const char* key)
{
    if(!valid_with_type(json, SK_OBJECT_NODE)) {
        THROW_ERR(WrongNodeType);
        return NULL;
    }

    return skHashTable_contains(json->data.j_object, key);
}
