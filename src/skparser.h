#ifndef __SK_PARSER_H__
#define __SK_PARSER_H__

#include "sknode.h"
#include "skscanner.h"
#include "skslice.h"

bool skJsonString_isvalid(const skStrSlice *slice);

skJsonNode *skJsonNode_parse(skScanner *scanner, skJsonNode *parent);

#endif
