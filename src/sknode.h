#ifndef __SK_NODE_H__
#define __SK_NODE_H__

#include "skhashtable.h"
#include "skscanner.h"
#include "sktypes.h"
#include "skvec.h"

/********* Json Element types **********/
typedef enum {
  SK_ERROR_NODE = 0,
  SK_OBJECT_NODE = 1,
  SK_ARRAY_NODE = 2,
  SK_STRING_NODE = 4,
  SK_STRINGLIT_NODE = 8,
  SK_INT_NODE = 16,
  SK_DOUBLE_NODE = 32,
  SK_BOOL_NODE = 64,
  SK_NULL_NODE = 128
} skNodeType;

/************ Json Data ***************/
typedef union skNodeData {
  skHashTable *j_object;
  skVec *j_array;
  skJsonString j_string;
  skJsonInteger j_int;
  skJsonDouble j_double;
  skJsonBool j_boolean;
  skJsonString j_err;
} skNodeData;

/********** Core type ***************/
typedef struct skJsonNode {
  skNodeType type;
  skNodeData data;
  struct skJsonNode *parent;
  size_t index;
} skJsonNode;

skJsonNode *skJsonObject_new(skJsonNode *parent);
skJsonNode *skJsonArray_new(skJsonNode *parent);
skJsonNode *skJsonNode_new(skScanner *scanner, skJsonNode *parent);
skJsonNode *skJsonError_new(skJsonString msg, skJsonState state,
                            skJsonNode *parent);
skJsonNode *skJsonString_new(skJsonString str, skJsonNode *parent);
skJsonNode *skJsonInteger_new(skJsonInteger number, skJsonNode *parent);
skJsonNode *skJsonDouble_new(skJsonDouble number, skJsonNode *parent);
skJsonNode *skJsonBool_new(skJsonBool boolean, skJsonNode *parent);
skJsonNode *skJsonNull_new(skJsonNode *parent);

void skJsonMember_drop(skJsonNode *member);
void skJsonNode_drop(skJsonNode *);

/* Debug */
void print_node(skJsonNode *node);

#endif
