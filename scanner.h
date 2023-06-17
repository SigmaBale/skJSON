#ifndef __SK_SCANNER_H__
#define __SK_SCANNER_H__

#include "token.h"
#include <stdbool.h>

typedef struct {
  Sk_CharIter iter;
  Sk_Token next;
} Sk_Scanner;

Sk_Scanner Sk_Scanner_new(Sk_CharIter iterator);

Sk_Token Sk_Scanner_next(Sk_Scanner *scanner);

Sk_Token Sk_Scanner_peek(const Sk_Scanner *scanner);

#endif
