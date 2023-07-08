#ifndef __SK_PARSER_H__
#define __SK_PARSER_H__

#include "sknode.h"
#include "skscanner.h"
#include "skslice.h"

bool skJsonString_isvalid(const skStrSlice *slice);

skJson skJsonNode_parse(skScanner *scanner, skJson *parent);

#endif
