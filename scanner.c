#include "scanner.h"
#include <stdio.h>

static const Token EmptyToken = { 0 };

/*
  JSON_OBJECT,
  JSON_ARRAY,
  JSON_STRING,
  JSON_INT_VAL,
  JSON_DOUBLE_VAL,
  JSON_BOOL,
  JSON_NULL,
 */

/*
    typedef struct {
      CharIterator iter;
      Token next;
    } Scanner;
 */
Scanner
scanner_new(CharIterator iterator)
{
    return (Scanner) {
        .iter = iterator,
        .next = EmptyToken,
    };
}

bool
scanner_has_next(const Scanner* scanner)
{
    if(scanner == NULL || iterator_char_peek_next(&scanner->iter) == EOF) {
        return false;
    } else {
        return true;
    }
}

Token
scanner_next(Scanner* scanner)
{
    // TODO
    return EmptyToken;
}
