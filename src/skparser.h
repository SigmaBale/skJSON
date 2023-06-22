#ifndef __SK_JSON_H__
#define __SK_JSON_H__

#include "sknode.h"
#include "skscanner.h"
#include "sktoken.h"
#include <stddef.h>

typedef struct skJson skJson;

skJson *sk_json_new(void *buff, size_t bufsize);

skJsonString skJsonString_new(skToken token);
skJsonNode *skparse_json_object(skScanner *scanner, skJsonNode *parent);
skJsonNode *skparse_json_array(skScanner *scanner, skJsonNode *parent);
skJsonNode *skparse_json_string(skScanner *scanner, skJsonNode *parent);
skJsonNode *skparse_json_number(skScanner *scanner, skJsonNode *parent);
skJsonNode *skparse_json_bool(skScanner *scanner, skJsonNode *parent);
skJsonNode *skparse_json_null(skScanner *scanner, skJsonNode *parent);

#endif
