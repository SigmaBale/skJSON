#include "scanner.h"
#include <stdio.h>

Sk_Scanner
Sk_Scanner_new(Sk_CharIter iterator)
{
    return (Sk_Scanner) {
        .iter = iterator,
        .next = (Sk_Token) { 0 },
    };
}

bool
Sk_Scanner_has_next(const Sk_Scanner* scanner)
{
    if(scanner == NULL || iterator_char_peek_next(&scanner->iter) == EOF) {
        return false;
    } else {
        return true;
    }
}

Sk_Token
Sk_Scanner_next(Sk_Scanner* scanner)
{
    // TODO
    return (Sk_Token) { 0 };
}
