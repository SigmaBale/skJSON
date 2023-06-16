#include "scanner.h"
#include <stdint.h>

typedef const char *JsonError;

typedef enum {
  JERROR_NODE = -1,
  JOBJECT_NODE = 0,
  JARRAY_NODE = 1,
  JSTRING_NODE = 2,
  JINT_NODE = 3,
  JDOUBLE_NODE = 4,
  JBOOL_NODE = 5,
  JNULL_NODE = 6
} NodeType;

typedef struct JsonObject JsonObject;
typedef struct JsonArray JsonArray;
typedef int64_t JsonInteger;
typedef char *JsonString;
typedef double JsonDouble;
typedef bool JsonBool;
typedef void *JsonNull;

typedef union {
  JsonError err;
  JsonObject *j_object;
  JsonArray *j_array;
  JsonString j_string;
  JsonInteger j_int;
  JsonDouble j_double;
  JsonBool j_boolean;
  JsonNull null;
} NodeData;

typedef struct {
  NodeType type;
  NodeData data;
} JsonNode;

struct JsonArray {
  JsonNode **values;
};

typedef struct {
  JsonString string;
  JsonNode value;
} JsonMember;

struct JsonObject {
  JsonMember **members;
};

// TODO: implement these
JsonNode *JsonErrorNode_new(const char *msg);
JsonNode *JsonObjectNode_new(JsonMember **members);
JsonNode *JsonArrayNode_new(JsonNode **values);
JsonNode *JsonStringNode_new(JsonString str);
JsonNode *JsonIntegerNode_new(JsonInteger number);
JsonNode *JsonDoubleNode_new(JsonDouble number);
JsonNode *JsonBoolNode_new(JsonBool boolean);
JsonNode *JsonNullNode_new(void);

// TODO: implement these
JsonNode *parse_json_object(Scanner *scanner);
JsonNode *parse_json_array(Scanner *scanner);
JsonNode *parse_json_string(Scanner *scanner);
JsonNode *parse_json_number(Scanner *scanner);
JsonNode *parse_json_true(Scanner *scanner);
JsonNode *parse_json_false(Scanner *scanner);
JsonNode *parse_json_null(Scanner *scanner);
JsonNode *json_error_node();
