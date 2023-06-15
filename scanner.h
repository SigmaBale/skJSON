#ifndef __SCANNER_H__
#define __SCANNER_H__

#include "token.h"
#include <stdbool.h>

typedef struct {
  CharIterator iter;
  Token token;
} Scanner;

Scanner scanner_new(CharIterator iterator);

bool scanner_has_next(const Scanner *scanner);

Token scanner_next(Scanner *scanner);

Token scanner_peek(const Scanner *scanner);

#endif
