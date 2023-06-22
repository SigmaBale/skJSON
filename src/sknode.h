#ifndef __SK_NODE_H__
#define __SK_NODE_H__

// clang-format off
#include "skhashtable.h"
#include "skscanner.h"
#include <stdint.h>
#include "skvec.h"
// clang-format on

#define TABLE_SIZE_LIMIT                                                       \
  fprintf(stderr, "error %s:%d: table size limit exceeded\n", __FILE__,        \
          __LINE__)

#define INVALID_KEY                                                            \
  fprintf(stderr, "error %s:%d: invalid key value (nill)\n", __FILE__, __LINE__)

#define ZST_NOT_ALLOWED                                                        \
  fprintf(stderr, "error %s:%d: zero sized types not allowed\n", __FILE__,     \
          __LINE__)

#define PRINT_OOM_ERR                                                          \
  fprintf(stderr, "error %s:%d: out of memory\n", __FILE__, __LINE__)

#define is_null(object) (object) == NULL

#define null_check_with_ret(object, ret)                                       \
  if ((object) == NULL) {                                                      \
    return (ret);                                                              \
  }

#define null_check_with_err_and_ret(object, err, ret)                          \
  if ((object) == NULL) {                                                      \
    err;                                                                       \
    return (ret);                                                              \
  }

#define null_check_oom_err(object)                                             \
  if ((object) == NULL) {                                                      \
    PRINT_OOM_ERR;                                                             \
    return NULL;                                                               \
  }

#define check_with_ret(exp, ret)                                               \
  if ((exp))                                                                   \
    return ret;

#define check_with_err_and_ret(exp, err, ret)                                  \
  if ((exp)) {                                                                 \
    err;                                                                       \
    return ret;                                                                \
  }

#define print_err_and_ret(err, ret)                                            \
  err;                                                                         \
  return ret

/********* Json Element types **********/
typedef enum {
  SK_ERROR_NODE = -1,
  SK_OBJECT_NODE = 0,
  SK_ARRAY_NODE = 1,
  SK_STRING_NODE = 2,
  SK_EMPTYSTRING_NODE = 3,
  SK_INT_NODE = 4,
  SK_DOUBLE_NODE = 5,
  SK_BOOL_NODE = 6,
  SK_NULL_NODE = 7
} skNodeType;
/**************************************/
//
//
//
/************** Union *****************/
typedef union skNodeData skNodeData;
/********** Union types ***************/
typedef const char *skJsonError;
typedef int64_t skJsonInteger;
typedef char *skJsonString;
typedef double skJsonDouble;
typedef bool skJsonBool;
union skNodeData {
  skJsonError j_err;
  skHashTable *j_object;
  skVec *j_array;
  skJsonString j_string;
  skJsonInteger j_int;
  skJsonDouble j_double;
  skJsonBool j_boolean;
};
/**************************************/
//
//
//
/********** Core type ***************/
typedef struct skJsonNode skJsonNode;
struct skJsonNode {
  skNodeType type;
  skNodeData data;
  skJsonNode *parent;
  long int index;
};
/************************************/
//
//
//
/******** Json Object member ********/
typedef struct {
  skJsonString string;
  skJsonNode *value;
} skJsonMember;
/************************************/
//
//
//
skJsonNode *skJsonObject_new(skJsonNode *parent);
skJsonNode *skJsonArray_new(skJsonNode *parent);
skJsonNode *skJsonNode_new(skScanner *scanner, skJsonNode *parent);
skJsonNode *skJsonErrorNode_new(skJsonError msg, skJsonNode *parent);
skJsonNode *skJsonStringNode_new(skJsonString str, skJsonNode *parent);
skJsonNode *skJsonInteger_new(skJsonInteger number, skJsonNode *parent);
skJsonNode *skJsonDouble_new(skJsonDouble number, skJsonNode *parent);
skJsonNode *skJsonBool_new(skJsonBool boolean, skJsonNode *parent);
skJsonNode *skJsonNull_new(skJsonNode *parent);

void skJsonMember_drop(skJsonMember *member);
void skJsonNode_drop(skJsonNode *);

void print_node(skJsonNode *node);

#endif
