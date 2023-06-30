#ifndef __SK_NODE_H__
#define __SK_NODE_H__

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
  skVec *j_object;
  skVec *j_array;
  skJsonString j_string;
  skJsonInteger j_int;
  skJsonDouble j_double;
  skJsonBool j_boolean;
  skJsonString j_err;
} skNodeData;

/********** Core type ***************/
typedef struct skJsonNode skJsonNode;

struct skJsonNode {
  skNodeType type;
  skNodeData data;
  skJsonNode *parent;
  size_t index;
};

/******** Json Object Tuple **********/
typedef struct {
  char *key;
  skJsonNode value;
} skObjectTuple;

skJsonNode *RawNode_new(skNodeType type, skJsonNode *parent);
skJsonNode *ObjectNode_new(skJsonNode *parent);
skJsonNode *ArrayNode_new(skJsonNode *parent);
skJsonNode *StringNode_new(skJsonString str, skNodeType type, skJsonNode *parent);
skJsonNode *IntNode_new(skJsonInteger number, skJsonNode *parent);
skJsonNode *DoubleNode_new(skJsonDouble number, skJsonNode *parent);
skJsonNode *BoolNode_new(skJsonBool boolean, skJsonNode *parent);
skJsonNode *ErrorNode_new(skJsonString msg, skJsonState state,
                          skJsonNode *parent);
void skJsonNode_drop(skJsonNode *node);
void skObjectTuple_drop(skObjectTuple *tuple);

/* Debug */
void print_node(skJsonNode *node);

#endif
