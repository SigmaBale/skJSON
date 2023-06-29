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

typedef struct _SBuff SBuff;

PRIVATE(skJsonNode*) RawNode_new(skNodeType type, skJsonNode* parent);
PRIVATE(skJsonNode*) IntNode_new(long int n, skJsonNode* parent);
PRIVATE(skJsonNode*) DoubleNode_new(double n, skJsonNode* parent);
PRIVATE(skJsonNode*) BoolNode_new(bool b, skJsonNode* parent);
PRIVATE(skJsonNode*) StringNode_new(const char* string, skNodeType type);
PRIVATE(skJsonNode*) ObjectNode_new(skJsonNode* parent);
PRIVATE(skJsonNode*) ArrayNode_new(skJsonNode* parent);
PRIVATE(bool) json_serialize(skJsonNode* json, SBuff* buff);
PRIVATE(bool) serialize_number(SBuff* buffer, skJsonNode* json);
PRIVATE(bool) serialize_string(SBuff* buffer, skJsonNode* json);
PRIVATE(bool) serialize_bool(SBuff* buffer, skJsonNode* json);
PRIVATE(bool) serialize_null(SBuff* buffer);
PRIVATE(bool) serialize_array(SBuff* buffer, skJsonNode* json);
PRIVATE(bool) serialize_object(SBuff* buffer, skJsonNode* json);
PRIVATE(void) drop_nonprim_elements(skJsonNode* json);
PRIVATE(bool) array_push_node_checked(skJsonNode* json, skJsonNode* node);

struct _SBuff {
    unsigned char* buffer;
    size_t         length;
    size_t         offset;
    size_t         depth;
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
drop_nonprim_elements(skJsonNode* json) {
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

    drop_nonprim_elements(json);

    json->data.j_object = object;
    json->type = SK_ARRAY_NODE;

    return json;
}

PUBLIC(skJsonNode*)
skJson_integer_new(long int n) {
    return IntNode_new(n, NULL);
}

PRIVATE(skJsonNode*)
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

PRIVATE(skJsonNode*)
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
    return BoolNode_new(boolean, NULL);
}

PRIVATE(skJsonNode*)
BoolNode_new(bool b, skJsonNode* parent)
{
    skJsonNode* bool_node;

    bool_node = RawNode_new(SK_BOOL_NODE, parent);
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
    return RawNode_new(SK_NULL_NODE, NULL);
}

PRIVATE(skJsonNode*)
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

/* Try creating a valid Json String from 'string' that holds the newly allocated
 * duplicated string */
PUBLIC(skJsonNode*)
skJson_string_new(const char* string)
{
    return StringNode_new(string, SK_STRING_NODE);
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
    return StringNode_new(string, SK_STRINGLIT_NODE);
}

/* Check validity and construct the valid Json string from 'string' of type 'type' */
PRIVATE(skJsonNode*)
StringNode_new(const char* string, skNodeType type)
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
    string_node = RawNode_new(type, NULL);
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
    return ArrayNode_new(NULL);
}

PRIVATE(skJsonNode*)
ArrayNode_new(skJsonNode* parent)
{
    skJsonNode* array_node;
    skVec* array;

    if(is_null(array_node = RawNode_new(SK_ARRAY_NODE, parent))) {
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

/* Insert NULL at the index 'index' into the 'json', 'json' being the valid Json Array */
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



/* Create Json Array of Json strings */
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

/* Create Json Array of references to Json strings */
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

/* Create Json Array of Json integers */
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

/* Create Json Array of Json doubles */
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

/* Create Json Array of Json Booleans */
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

/* Create Json Array of NULL values */
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
    return ObjectNode_new(NULL);
}

PRIVATE(skJsonNode*)
ObjectNode_new(skJsonNode* parent)
{
    skJsonNode* object_node;

    object_node = RawNode_new(SK_OBJECT_NODE, parent);

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

PRIVATE(SBuff)
SBuff_new(void)
{
    static const size_t buffer_size = BUFSIZ;

    SBuff pbuf;
    unsigned char* buffer;

    memset(&pbuf, 0, sizeof(SBuff));

    if(is_null(buffer = malloc(buffer_size))) {
        THROW_ERR(OutOfMemory);
        return pbuf;
    }

    pbuf.length = buffer_size;
    pbuf.buffer = buffer;

    return pbuf;
}

PRIVATE(void)
SBuff_drop(SBuff* buff)
{
#ifdef SK_DBUG
    assert(is_some(buff));
    assert(is_some(buff->buffer));
#endif
    free(buff->buffer);
    memset(buff, 0, sizeof(SBuff));
}

PRIVATE(unsigned char*)
SBuff_ensure(SBuff* buff, size_t needed)
{
#ifdef SK_DBUG
    assert(is_some(buff));
    assert(is_some(buff->buffer));
    assert(!(buf->length > 0 && buf->offset >= buf->length));
#endif
    unsigned char* newbuf;
    size_t newsize;

    if(needed > INT_MAX) {
        THROW_ERR(AllocationTooLarge);
        return NULL;
    }

    needed += buff->offset + 1; /* 1 for null terminator */

    /* If we got enough space return current position */
    if(needed <= buff->length) {
        return buff->buffer + buff->offset;
    }

    /* Else try expand the buffer */
    if(needed > (INT_MAX/2)) {
        if(needed <= INT_MAX) {
            newsize = INT_MAX;
        } else {
            THROW_ERR(AllocationTooLarge);
            return NULL;
        }
    } else {
        newsize = needed * 2;
    }

    /* Try reallocate the buffer to 'newsize' */
    newbuf = realloc(buff->buffer, newsize);

    if(is_null(newbuf)) {
        THROW_ERR(OutOfMemory);
        SBuff_drop(buff);
        return NULL;
    }

    buff->buffer = newbuf;
    buff->length = newsize;

    return newbuf + buff->offset;
}

PRIVATE(void)
SBuff_update_offset(SBuff* buff)
{
#ifdef SK_DBUG
    assert(is_some(buff));
    assert(is_some(buff->buffer));
#endif
    buff->offset += strlen((char*)buff->buffer + buff->offset);
}

PUBLIC(unsigned char*)
skJson_serialize(skJsonNode* json) {
    SBuff pbuf;

    if(is_null(json)) {
        return NULL;
    }

    pbuf = SBuff_new();

    if(is_null((pbuf = SBuff_new()).buffer)) {
        return NULL;
    }
    
    if(!json_serialize(json, &pbuf)) {
        THROW_ERR(SerializerError);
        return NULL;
    }

    return pbuf.buffer;
}

PRIVATE(bool)
json_serialize(skJsonNode* json, SBuff* sbuf)
{
#ifdef SK_DBUG
    assert(is_some(json));
    assert(is_some(sbuf));
    assert(is_some(sbuf->buffer));
#endif
    switch(json->type) {
        case SK_STRING_NODE:
        case SK_STRINGLIT_NODE:
            return serialize_string(sbuf, json);
        case SK_INT_NODE:
        case SK_DOUBLE_NODE:
            return serialize_number(sbuf, json);
        case SK_BOOL_NODE:
            return serialize_bool(sbuf, json);
        case SK_NULL_NODE:
            return serialize_null(sbuf);
        case SK_ARRAY_NODE:
            return serialize_array(sbuf, json);
        case SK_OBJECT_NODE:
            return serialize_object(sbuf, json);
        case SK_ERROR_NODE:
        default:
            return false;
    }
}

PRIVATE(bool)
serialize_number(SBuff *buffer, skJsonNode* json)
{
#ifdef SK_DBUG
    assert(is_some(buffer));
    assert(is_some(buffer->buffer));
    assert(is_some(json));
#endif
    char buff[30];
    unsigned char* out;
    size_t len;

    if(json->type == SK_DOUBLE_NODE) {
        len = snprintf(buff, 30, "%lf", json->data.j_double);
    } else {
        len = snprintf(buff, 30, "%ld", json->data.j_int);
    }

    if(len < 0 || len >= 30) {
        return false;
    }

    out = SBuff_ensure(buffer, len + sizeof("")); /* len + '\0' */

    if(is_null(out)) {
        return false;
    }

    strcat((char*)out, buff);
    SBuff_update_offset(buffer);

    return true;
}

PRIVATE(bool)
serialize_string(SBuff *buffer, skJsonNode* json)
{
#ifdef SK_DBUG
    assert(is_some(buffer));
    assert(is_some(buffer->buffer));
    assert(is_some(json));
    assert(json->type == SK_STRING_NODE || json->type == SK_STRINGLIT_NODE);
#endif
    unsigned char* out;
    char* str;
    size_t len;

    len = strlen((str = json->data.j_string)) + sizeof("\"\"");
    out = SBuff_ensure(buffer, len);

    if(is_null(out)) {
        return false;
    }

    out[0] = '\"';
    memcpy(out + 1, str, len);
    out[len + 1] = '\"';
    out[len + 2] = '\0';

    SBuff_update_offset(buffer);

    return true;
}

PRIVATE(bool)
serialize_bool(SBuff *buffer, skJsonNode* json)
{
#ifdef SK_DBUG
    assert(is_some(buffer));
    assert(is_some(buffer->buffer));
    assert(is_some(json));
#endif
    unsigned char* out;
    bool boolean;

    boolean = json->data.j_boolean;

    if(boolean) {
        out = SBuff_ensure(buffer, 5);

        if(is_null(out)) {
            return false;
        }

        strcat((char*)out, "true");
    } else {
        out = SBuff_ensure(buffer, 6);

        if(is_null(out)) {
            return false;
        }

        strcat((char*)out, "false");
    }

    SBuff_update_offset(buffer);
    return true;
}

PRIVATE(bool)
serialize_null(SBuff *buffer)
{
#ifdef SK_DBUG
    assert(is_some(buffer));
    assert(is_some(buffer->buffer));
#endif
    unsigned char* out;

    if(is_null(out = SBuff_ensure(buffer, 5))) {
        return false;
    }

    strcat((char*)out, "null");
    SBuff_update_offset(buffer);

    return true;
}

PRIVATE(bool)
serialize_array(SBuff *buffer, skJsonNode* json)
{
#ifdef SK_DBUG
    assert(is_some(buffer));
    assert(is_some(buffer->buffer));
    assert(is_some(json));
    assert(json->type == SK_ARRAY_NODE);
#endif

    unsigned char* out;
    skVec* vec;
    size_t len, i;
    skJsonNode* current;

    if(is_null(out = SBuff_ensure(buffer, 1))) {
        return false;
    }

    out[0] = '[';
    buffer->offset++;
    buffer->depth++;

    vec = json->data.j_array;
    len = skVec_len(vec);

    for(i = 0; i < len; i++) {
        current = skVec_index(vec, i);
#ifdef SK_DBUG
        assert(is_some(current));
#endif
        if(!json_serialize(current, buffer)) {
            return false;
        }

        if(i + 1 < len) {
            if(is_null(out = SBuff_ensure(buffer, 2))) {
                return false;
            }
            *out++ = ',';
            *out = '\0';
            buffer->offset += 2;
        }
    }

    if(is_null(out = SBuff_ensure(buffer, 2))) {
        return false;
    }

    *out++ = ']';
    *out = '\0';
    buffer->depth--;
    buffer->offset += 2;

    return true;
}

PRIVATE(bool)
serialize_object(SBuff* buff, skJsonNode* json)
{
#ifdef SK_DBUG
    assert(is_some(buff));
    assert(is_some(buff->buffer));
    assert(is_some(json));
    assert(json->type == SK_OBJECT_NODE);
#endif

    unsigned char* out;
    skHashTable* table;
    size_t len, i;
    skJsonNode* current;

    if(is_null(out = SBuff_ensure(buff, 1))) {
        return false;
    }

    out[0] = '{';
    buff->offset++;
    buff->depth++;

    table = json->data.j_object;
    len = skHashTable_len(table);

    for(i = 0; i < len; i++) {

    }
}
