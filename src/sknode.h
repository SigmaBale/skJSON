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
  SK_NULL_NODE = 128,
  SK_MEMBER_NODE = 256
} skNodeType;

typedef struct _skJsonNode skJson;
typedef struct skJsonMember skJsonMember;

typedef struct {
  unsigned char *ptr;
  skNodeType type;
} skArena;

typedef union skNodeData {
  skVec *j_object;
  skVec *j_array;
  skJsonString j_string;
  skJsonInteger j_int;
  skJsonDouble j_double;
  skJsonBool j_boolean;
  skJsonString j_err;
} skNodeData;

struct _skJsonNode {
  skNodeType type;
  skNodeData data;
  skArena parent_arena;
};

typedef struct {
  skJson value;
  char *key;
} skObjTuple;

skJson RawNode_new(skNodeType type, const skJson *parent);
skJson ObjectNode_new(const skJson *parent);
skJson ArrayNode_new(const skJson *parent);
skJson StringNode_new(skJsonString str, skNodeType type, const skJson *parent);
skJson IntNode_new(skJsonInteger number, const skJson *parent);
skJson DoubleNode_new(skJsonDouble number, const skJson *parent);
skJson BoolNode_new(skJsonBool boolean, const skJson *parent);
skJson MemberNode_new(const char *key, const skJson *value,
                      const skJson *parent);
skJson ErrorNode_new(skJsonString msg, skJsonState state, const skJson *parent);
void skJsonNode_drop(skJson *node);
void skObjTuple_drop(skObjTuple *tuple);

#endif
