#ifndef __SK_SCANNER_H__
#define __SK_SCANNER_H__

#include "token.h"
#include <stdbool.h>
#include <stdio.h>

typedef struct {
  Sk_CharIter iter;
  Sk_Token next;
} Sk_Scanner;

extern char SK_JSON_BUF[BUFSIZ];

Sk_Scanner Sk_Scanner_new(Sk_CharIter iterator);

Sk_Token Sk_Scanner_next(Sk_Scanner *scanner);

Sk_Token Sk_Scanner_peek(const Sk_Scanner *scanner);

Sk_Token Sk_Scanner_skip(Sk_Scanner* scanner, Sk_TokenType type);

bool _sk_refill_buf(void);

#endif
