#ifndef __TOKEN_H__
#define __TOKEN_H__

#include "slice.h"

typedef enum {
  END_TOK,
  LCURLY_TOK,
  RCURLY_TOK,
  LBRACK_TOK,
  RBRACK_TOK,
  STRING_TOK,
  NUMBER_TOK,
  TRUE_TOK,
  FALSE_TOK,
  NULL_TOK,
} TokenType;

typedef struct {
  TokenType type;
  StrSlice lexeme;
} Token;

#endif
