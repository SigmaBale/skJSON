#ifndef __SK_NODE_H__
#define __SK_NODE_H__

#include "scanner.h"
#include "sk_vec.h"
#include <stdint.h>

#define PRINT_OOM_ERR                                                          \
  fprintf(stderr, "error %s:%d: out of memory\n", __FILE__, __LINE__)

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
} Sk_NodeType;

typedef union Sk_NodeData Sk_NodeData;
typedef const char *Sk_JsonError;
typedef struct Sk_JsonObject Sk_JsonObject;
typedef struct Sk_JsonArray Sk_JsonArray;
typedef int64_t Sk_JsonInteger;
typedef char *Sk_JsonString;
typedef double Sk_JsonDouble;
typedef bool Sk_JsonBool;
typedef void *Sk_JsonNull;
typedef const char *Sk_JsonEmptyString;

typedef struct {
  Sk_NodeType type;
  Sk_NodeData *data;
} Sk_JsonNode;

struct Sk_JsonArray {
  Sk_Vec nodes;
};

typedef struct {
  Sk_JsonString string;
  Sk_JsonNode *value;
} Sk_JsonMember;

struct Sk_JsonObject {
  Sk_Vec members;
};

union Sk_NodeData {
  Sk_JsonError j_err;
  Sk_JsonObject j_object;
  Sk_JsonArray j_array;
  Sk_JsonString j_string;
  Sk_JsonEmptyString j_empty_string;
  Sk_JsonInteger j_int;
  Sk_JsonDouble j_double;
  Sk_JsonBool j_boolean;
  Sk_JsonNull j_null;
};

Sk_JsonNode *Sk_JsonNode_new(Sk_Scanner *scanner);
Sk_JsonNode *Sk_JsonErrorNode_new(Sk_JsonError msg);
Sk_JsonNode *Sk_JsonObjectNode_new(Sk_Vec members);
Sk_JsonNode *Sk_JsonArrayNode_new(Sk_Vec nodes);
Sk_JsonNode *Sk_JsonStringNode_new(Sk_JsonString str, Sk_NodeType type);
Sk_JsonNode *Sk_JsonIntegerNode_new(Sk_JsonInteger number);
Sk_JsonNode *Sk_JsonDoubleNode_new(Sk_JsonDouble number);
Sk_JsonNode *Sk_JsonBoolNode_new(Sk_JsonBool boolean);
Sk_JsonNode *Sk_JsonNullNode_new(void);

void Sk_JsonArray_drop(Sk_JsonArray *array);
void Sk_JsonObject_drop(Sk_JsonObject *object);
void Sk_JsonMember_drop(Sk_JsonMember *member);
void Sk_JsonNode_drop(Sk_JsonNode *);

#endif
