#include "scanner.h"
#include "sk_vec.h"
#include <ctype.h>

char SK_JSON_BUF[BUFSIZ];

Sk_Scanner
Sk_Scanner_new(Sk_CharIter iterator)
{
    return (Sk_Scanner) {
        .iter = iterator,
        .next = (Sk_Token) { 0 },
    };
}

void static _skip_ws(Sk_Scanner* scanner)
{
    while(isspace(Sk_CharIter_next(&scanner->iter))) {
        ;
    }
}

Sk_Token
Sk_Scanner_skip(Sk_Scanner* scanner, Sk_TokenType type)
{
    Sk_Token token;

    while((token = Sk_Scanner_next(scanner)).type == type) {
        ;
    }

    return token;
}

int static _write_until(Sk_Scanner* scanner, char ch)
{
    int           c;
    static Sk_Vec vec = (Sk_Vec) {
        .len        = 0,
        .capacity   = 0,
        .ele_size   = sizeof(char),
        .allocation = NULL,
    };

    do {

        if((c = Sk_CharIter_next(&scanner->iter)) == ch) {
            break;
        } else if(c == '\n' || c == '\v') {
            return 1;
        } else {
            Sk_Vec_push(&vec, &c);
        }

    } while(c == EOF && _sk_refill_buf() == true);

    if(c == EOF) {
        return 1;
    }

    scanner->next.lexeme.ptr = Sk_Vec_front(&vec);
    scanner->next.lexeme.len = vec.len;

    /// Cheating a little, next _write_until will
    /// overwrite current vec data when we fetch next token
    vec.len = 0;

    return 0;
}

Sk_Token
Sk_Scanner_next(Sk_Scanner* scanner)
{
    int      c;
    Sk_Token current = scanner->next;

    _skip_ws(scanner);

    switch(Sk_CharIter_next(&scanner->iter)) {
        case '"':
            if(_write_until(scanner, '"') != 0) {
                scanner->next.type = SK_INVALID;
            }
            break;
        default:
            scanner->next.type = SK_INVALID;
            break;
    }

    return scanner->next;
}
