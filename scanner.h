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

/// Inlined and shared across multiple .c files
inline Sk_Token Sk_Scanner_peek(const Sk_Scanner* scanner) { return scanner->token; }

void Sk_Scanner_skip(Sk_Scanner *scanner, size_t n, ...);

#endif
