#ifndef __SK_SCANNER_H__
#define __SK_SCANNER_H__

#include <stdbool.h>
#include "sk_vec.h"
#include "token.h"
#include <stdio.h>

typedef struct {
  Sk_CharIter iter;
  Sk_Token token;
} Sk_Scanner;

Sk_Scanner *Sk_Scanner_new(void *buffer, size_t bufsize);

Sk_Token Sk_Scanner_next(Sk_Scanner *scanner);

Sk_Token Sk_Scanner_peek(const Sk_Scanner *scanner);

void Sk_Scanner_skip(Sk_Scanner *scanner, Sk_TokenType type);

#endif
