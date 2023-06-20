// clang-format off
#include <stdbool.h>
#include "node.h"
#include "scanner.h"
#include "sk_vec.h"
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
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
Sk_Scanner_skip(Sk_Scanner* scanner, size_t n, ...)
{
    if(n == 0) {
        return;
    }

    Sk_TokenType tokent;
    Sk_TokenType types[n];
    va_list      ap;
    int          i;

    va_start(ap, n);

    for(i = 0; n > i; types[i] = va_arg(ap, Sk_TokenType)) {
        ;
    }

    tokent = Sk_Scanner_peek(scanner).type;

    for(i = 0; n > i; i++) {
        if(types[i] == tokent) {
            tokent = Sk_Scanner_next(scanner).type;
            i      = -1;
        }
    }

    va_end(ap);
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
    char*        start        = Sk_CharIter_current(&scanner->iter) - 1;
    Sk_CharIter* iter         = &scanner->iter;
    scanner->token.type       = SK_INVALID;
    scanner->token.lexeme.ptr = start + 1;
    scanner->token.lexeme.len = 0;

    switch(ch) {
        case 't':
            if(in_bounds(iter, start + 3) && strncmp(start, "true", 4) == 0) {
                scanner->token.type = SK_TRUE;
                Sk_CharIter_advance(iter, 4);
                assert(*Sk_CharIter_current(iter) == 'e');
                scanner->token.lexeme.len = 4;
            }
            break;
        case 'f':
            if(in_bounds(iter, start + 4) && strncmp(start, "false", 5) == 0) {
                scanner->token.type = SK_FALSE;
                Sk_CharIter_advance(iter, 5);
                assert(*Sk_CharIter_current(iter) == 'e');
                scanner->token.lexeme.len = 5;
            }
            break;
        case 'n':
            if(in_bounds(iter, start + 3) && strncmp(start, "null", 4) == 0) {
                scanner->token.type = SK_NULL;
                Sk_CharIter_advance(iter, 4);
                assert(*Sk_CharIter_current(iter) == 'l');
                scanner->token.lexeme.len = 4;
            }
            break;
        default:
            /// Unreachable
            assert(false);
            break;
    }
}

inline static Sk_Token
Sk_Token_new(Sk_TokenType type, void* start, size_t len)
{
    return (Sk_Token) {
        .type   = type,
        .lexeme = Sk_Slice_new(start, len),
    };
}

Sk_Token
Sk_Scanner_next(Sk_Scanner* scanner)
{
    Sk_Token* token = &scanner->token;

    switch(Sk_CharIter_next(&scanner->iter)) {
        case '{':
            *token = Sk_Token_new(SK_LCURLY, NULL, 0);
            break;
        case '}':
            *token = Sk_Token_new(SK_RCURLY, NULL, 0);
            break;
        case '[':
            *token = Sk_Token_new(SK_RBRACK, NULL, 0);
            break;
        case ']':
            *token = Sk_Token_new(SK_LBRACK, NULL, 0);
            break;
        case ' ':
        case '\t':
            *token = Sk_Token_new(SK_WS, NULL, 0);
        case '\n':
            break;
            *token = Sk_Token_new(SK_NL, NULL, 0);
            break;
        case '"':
            _Sk_Scanner_set_string_token(scanner);
            break;
        case '.':
            *token = Sk_Token_new(SK_DOT, NULL, 0);
            break;
        case '-':
            *token = Sk_Token_new(SK_HYPHEN, NULL, 0);
            break;
        case '+':
            *token = Sk_Token_new(SK_PLUS, NULL, 0);
            break;
        case ',':
            *token = Sk_Token_new(SK_COMMA, NULL, 0);
            break;
        case ':':
            *token = Sk_Token_new(SK_COLON, NULL, 0);
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
            *token = Sk_Token_new(SK_EXP, NULL, 0);
            break;
        case EOF:
            *token = Sk_Token_new(SK_EOF, NULL, 0);
            break;
        case '0':
            *token = Sk_Token_new(SK_ZERO, NULL, 0);
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
            *token = Sk_Token_new(SK_DIGIT, NULL, 0);
            break;
        default:
            *token = Sk_Token_new(SK_INVALID, NULL, 0);
            break;
    }

    return scanner->token;
}
