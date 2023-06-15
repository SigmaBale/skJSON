#include "scanner.h"
#include <stdio.h>

static Token NullToken = { .type = NONE, .lexeme = NULL };

Scanner
scanner_new(CharIterator iterator)
{
    return (Scanner) {
        .iter  = iterator,
        .token = NONE,
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
    return NullToken;
}
