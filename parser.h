#ifndef __SK_PARSER_H__
#define __SK_PARSER_H__

#include "node.h"
#include "scanner.h"

typedef struct Sk_Json Sk_Json;

Sk_Json *sk_parse_json(char *json_file);

static Sk_JsonString Sk_JsonString_new(Sk_Token token);
static Sk_JsonNode *Sk_parse_json_object(Sk_Scanner *scanner);
static Sk_JsonNode *Sk_parse_json_array(Sk_Scanner *scanner);
static Sk_JsonNode *Sk_parse_json_string(Sk_Scanner *scanner);
static Sk_JsonNode *Sk_parse_json_number(Sk_Scanner *scanner);
static Sk_JsonNode *Sk_parse_json_bool(Sk_Scanner *scanner);
static Sk_JsonNode *Sk_parse_json_null(Sk_Scanner *scanner);

#endif
