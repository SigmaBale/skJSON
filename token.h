#ifndef __SK_TOKEN_H__
#define __SK_TOKEN_H__

#include "slice.h"

typedef enum {
  SK_END,
  SK_LCURLY,
  SK_RCURLY,
  SK_LBRACK,
  SK_RBRACK,
  SK_STRING,
  SK_NUMBER,
  SK_BOOL,
  SK_NULL,
  SK_COMMA,
  SK_SEMICOLON,
} Sk_TokenType;

typedef struct {
  Sk_TokenType type;
  Sk_StrSlice lexeme;
} Sk_Token;

#endif
