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

Sk_Token
Sk_Scanner_next(Sk_Scanner* scanner)
{

    // TODO
    return (Sk_Token) { 0 };
}
