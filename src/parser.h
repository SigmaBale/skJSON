#ifndef __SK_JSON_H__
#define __SK_JSON_H__

#include <stddef.h>
#include "token.h"
#include "scanner.h"
#include "node.h"

typedef struct skJson skJson;

skJson *sk_json_new(void *buff, size_t bufsize);

Sk_JsonString Sk_JsonString_new(Sk_Token token);
Sk_JsonNode*  Sk_parse_json_object(Sk_Scanner* scanner);
Sk_JsonNode*  Sk_parse_json_array(Sk_Scanner* scanner);
Sk_JsonNode*  Sk_parse_json_string(Sk_Scanner* scanner);
Sk_JsonNode*  Sk_parse_json_number(Sk_Scanner* scanner);
Sk_JsonNode*  Sk_parse_json_bool(Sk_Scanner* scanner);
Sk_JsonNode*  Sk_parse_json_null();

#endif
