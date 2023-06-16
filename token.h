#ifndef __SK_TOKEN_H__
#define __SK_TOKEN_H__

#include "slice.h"

typedef enum {
  SK_END_TOK,
  SK_LCURLY_TOK,
  SK_RCURLY_TOK,
  SK_LBRACK_TOK,
  SK_RBRACK_TOK,
  SK_QUOTES_TOK,
  SK_HYPHEN_TOK,
  SK_DIGIT_TOK,
  SK_TRUE_TOK,
  SK_FALSE_TOK,
  SK_NULL_TOK,
  SK_COMMA_TOK,
  SK_SEMICOLON_TOK,
} Sk_TokenType;

typedef struct {
  Sk_TokenType type;
  Sk_StrSlice lexeme;
} Sk_Token;

#endif
