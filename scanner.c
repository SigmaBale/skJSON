// clang-format off
#include <stdbool.h>
#include "node.h"
#include "scanner.h"
#include "sk_vec.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
// clang-format on

Sk_Scanner*
Sk_Scanner_new(void* buffer, size_t bufsize)
{
    if(buffer == NULL || bufsize == 0) {
        return NULL;
    }

    Sk_Scanner* scanner = malloc(sizeof(Sk_Scanner));

    if(scanner == NULL) {
        PRINT_OOM_ERR;
        return NULL;
    }

    scanner->iter  = Sk_CharIter_new(buffer, bufsize - 1);
    scanner->token = (Sk_Token) { 0 };

    return scanner;
}

inline void
Sk_Scanner_skip(Sk_Scanner* scanner, Sk_TokenType type)
{
    /// Peek and check next token type
    while(Sk_Scanner_peek(scanner).type == type) {
        /// Type matched lets advance
        Sk_Scanner_next(scanner);
    }
    /// Next call to 'Sk_Scanner_next' will yield new token type
}

void static _Sk_Scanner_set_string_token(Sk_Scanner* scanner)
{
    int          c;
    size_t       len;
    Sk_Token*    token    = &scanner->token;
    Sk_CharIter* iterator = &scanner->iter;

    /// Consume the first char '"'
    Sk_CharIter_next(iterator);

    token->type       = SK_STRING;
    token->lexeme.ptr = Sk_CharIter_current(iterator);
    token->lexeme.len = 0;

    /// Advance iterator until we hit closing quotes
    for(len = 0; (c = Sk_CharIter_next(&scanner->iter)) != '"'; len++) {
        if(c == EOF) {
            /// We reached end of file and string is invalid
            scanner->token.type = SK_INVALID;
            break;
        }
    }

    /// String is properly enclosed
    token->lexeme.len = len;
}

#define in_bounds(iter, bound) (iter)->end >= (bound)

void static _Sk_Scanner_set_bool_or_null_token(Sk_Scanner* scanner, char ch)
{
    /// Start one spot before the starting letter
    char*        start  = Sk_CharIter_current(&scanner->iter) - 1;
    Sk_CharIter* iter   = &scanner->iter;
    scanner->token.type = SK_INVALID;

    switch(ch) {
        case 't':
            if(in_bounds(iter, start + 3) && strncmp(start, "true", 4) == 0) {
                scanner->token.type = SK_TRUE;
                Sk_CharIter_advance(iter, 4);
                assert(*Sk_CharIter_current(iter) == 'e');
            }
            break;
        case 'f':
            if(in_bounds(iter, start + 4) && strncmp(start, "false", 5) == 0) {
                scanner->token.type = SK_FALSE;
                Sk_CharIter_advance(iter, 5);
                assert(*Sk_CharIter_current(iter) == 'e');
            }
            break;
        case 'n':
            if(in_bounds(iter, start + 3) && strncmp(start, "null", 4) == 0) {
                scanner->token.type = SK_NULL;
                Sk_CharIter_advance(iter, 4);
                assert(*Sk_CharIter_current(iter) == 'l');
            }
            break;
        default:
            /// Unreachable
            assert(1 == 2);
            break;
    }

    char* current;
    /// If next character is not EOF or space, then this is invalid token
    if((current = Sk_CharIter_current(iter) + 1) < iter->end && !isspace(*current)) {
        scanner->token.type = SK_INVALID;
    }
}

Sk_Token
Sk_Scanner_next(Sk_Scanner* scanner)
{
    switch(Sk_CharIter_peek(&scanner->iter)) {
        case '{':
            scanner->token.type = SK_LCURLY;
            break;
        case '}':
            scanner->token.type = SK_RCURLY;
            break;
        case '[':
            scanner->token.type = SK_RBRACK;
            break;
        case ']':
            scanner->token.type = SK_LBRACK;
            break;
        case ' ':
        case '\t':
        case '\n':
            scanner->token.type = SK_SPACE;
            break;
        case '"':
            _Sk_Scanner_set_string_token(scanner);
            break;
        case '.':
            scanner->token.type = SK_DOT;
            break;
        case '-':
            scanner->token.type = SK_HYPHEN;
            break;
        case '+':
            scanner->token.type = SK_PLUS;
            break;
        case ',':
            scanner->token.type = SK_COMMA;
            break;
        case ':':
            scanner->token.type = SK_COLON;
            break;
        case 't':
            _Sk_Scanner_set_bool_or_null_token(scanner, 't');
            break;
        case 'f':
            _Sk_Scanner_set_bool_or_null_token(scanner, 'f');
            break;
        case 'n':
            _Sk_Scanner_set_bool_or_null_token(scanner, 'n');
            break;
        case 'e':
        case 'E':
            scanner->token.type = SK_EXP;
            break;
        case EOF:
            scanner->token.type = SK_EOF;
            break;
        case '0':
            scanner->token.type = SK_ZERO;
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            scanner->token.type = SK_DIGIT;
            break;
        default:
            scanner->token.type = SK_INVALID;
            break;
    }

    /// Advance iterator
    Sk_CharIter_next(&scanner->iter);
    return scanner->token;
}
