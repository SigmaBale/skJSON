#ifndef __SK_SCANNER_H__
#define __SK_SCANNER_H__

// clang-format off
#include <stdbool.h>
#include "sk_vec.h"
#include "token.h"
#include <stdio.h>
// clang-format on 

typedef struct {
  Sk_CharIter iter;
  Sk_Token token;
} Sk_Scanner;

Sk_Scanner *Sk_Scanner_new(void *buffer, size_t bufsize);

Sk_Token Sk_Scanner_next(Sk_Scanner *scanner);

Sk_Token Sk_Scanner_peek(const Sk_Scanner* scanner);

void Sk_Scanner_skip(Sk_Scanner *scanner, size_t n, ...);

void Sk_Scanner_skip_until(Sk_Scanner* scanner, size_t n, ...);

void Sk_Token_print(Sk_Token token);

#endif
