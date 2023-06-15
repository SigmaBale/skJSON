#ifndef __TOKEN_H__
#define __TOKEN_H__

#include "slice.h"

typedef enum {
  L_PARAN,   /* '    (      ' */
  R_PARAN,   /* '    )      ' */
  L_CURLY,   /* '    {      ' */
  R_CURLY,   /* '    }      ' */
  L_BRACK,   /* '    [      ' */
  R_BRACK,   /* '    ]      ' */
  DOT,       /* '    .      ' */
  COMMA,     /* '    ,      ' */
  SEMICOLON, /* '    ;      ' */
  COLON,     /* '    :      ' */
  ASSIGN,    /* '    =      ' */
  EQ,        /* '    ==     ' */
  GT,        /* '    >      ' */
  LT,        /* '    <      ' */
  GTE,       /* '    >=     ' */
  LTE,       /* '    <=     ' */
  OR,        /* '    ||     ' */
  AND,       /* '    &&     ' */
  BOR,       /* '    |      ' */
  BAND,      /* '    &      ' */
  STAR,      /* '    *      ' */
  FSLASH,    /* '    /      ' */
  BSLASH,    /* '    \      ' */
  DQUOTE,    /* '    "      ' */
  SQUOTE,    /* '    '      ' */
  CHAR,      /* 'a-z or A-Z ' */
  DIGIT,     /* '   0-9     ' */
  INTEGER,   /* ' 1234...   ' */
  IDENT,     /* '_?SomeName ' */
  QMARK,     /* '    ?      ' */
  MINUS,     /* '    -      ' */
  PLUS,      /* '    +      ' */
  NONE,      /*     ???       */
} TokenType;

typedef struct {
  TokenType type;
  StrSlice lexeme;
} Token;

#endif
