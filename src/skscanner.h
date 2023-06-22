#ifndef __SK_SCANNER_H__
#define __SK_SCANNER_H__

// clang-format off
#include <stdbool.h>
#include "skvec.h"
#include "sktoken.h"
#include <stdio.h>
// clang-format on 

typedef struct {
  skCharIter iter;
  skToken token;
} skScanner;

skScanner *skScanner_new(void *buffer, size_t bufsize);

skToken skScanner_next(skScanner *scanner);

skToken skScanner_peek(const skScanner* scanner);

void skScanner_skip(skScanner *scanner, size_t n, ...);

void skScanner_skip_until(skScanner* scanner, size_t n, ...);

void skToken_print(skToken token);

#endif
