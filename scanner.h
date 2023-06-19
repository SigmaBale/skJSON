#ifndef __SK_SCANNER_H__
#define __SK_SCANNER_H__

#include "token.h"
#include <stdbool.h>
#include <stdio.h>

typedef struct {
  Sk_CharIter iter;
  Sk_Token token;
  char *buffer;
  char *ptr;
  size_t bufsize;
  int fd;
} Sk_Scanner;

Sk_Scanner* Sk_Scanner_new(int fd, size_t bufsize);

Sk_Token Sk_Scanner_next(Sk_Scanner *scanner);

Sk_Token Sk_Scanner_peek(const Sk_Scanner *scanner);

void Sk_Scanner_skip(Sk_Scanner *scanner, Sk_TokenType type);

#endif
