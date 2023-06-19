#ifndef __SK_PARSER_H__
#define __SK_PARSER_H__

// clang-format off
#include "scanner.h"
#include "node.h"
// clang-format on

typedef struct skJson skJson;

skJson *sk_json_new(void *buff, size_t bufsize);

#endif
