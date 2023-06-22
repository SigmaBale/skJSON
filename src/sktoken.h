#ifndef __SK_TOKEN_H__
#define __SK_TOKEN_H__

#include "skslice.h"

typedef enum {
  SK_INVALID,
  SK_LCURLY,
  SK_RCURLY,
  SK_LBRACK,
  SK_RBRACK,
  SK_WS,
  SK_NL,
  SK_STRING,
  SK_EOF,
  SK_DIGIT,
  SK_ZERO,
  SK_DOT,
  SK_HYPHEN,
  SK_EXP,
  SK_PLUS,
  SK_TRUE,
  SK_FALSE,
  SK_NULL,
  SK_COMMA,
  SK_COLON,
} skTokenType;

typedef struct {
  skTokenType type;
  skStrSlice lexeme;
} skToken;

#endif
