#ifndef __SK_PARSER_H__
#define __SK_PARSER_H__

#include "node.h"
#include "scanner.h"

Sk_JsonNode *sk_parse_json(Sk_Scanner *scanner);

#endif
