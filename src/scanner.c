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

Sk_Token
Sk_Scanner_peek(const Sk_Scanner* scanner)
{
    return scanner->token;
}

void
Sk_Scanner_skip(Sk_Scanner* scanner, size_t n, ...)
{
    if(n == 0) {
        return;
    }

    Sk_Token     token;
    Sk_TokenType types[n];
    va_list      ap;
    unsigned int i;

    va_start(ap, n);

    for(i = 0; n > i; i++) {
        types[i] = va_arg(ap, Sk_TokenType);
    }

    token = Sk_Scanner_next(scanner);

    for(i = 0; n > i; i++) {
        if(types[i] == token.type) {
            if((token = Sk_Scanner_next(scanner)).type == SK_EOF) {
                return;
            }
            i = -1;
        }
    }

    va_end(ap);
}

void
Sk_Scanner_skip_until(Sk_Scanner* scanner, size_t n, ...)
{
    if(n == 0) {
        return;
    }

    Sk_Token     token;
    Sk_TokenType types[n];
    va_list      ap;
    unsigned int i;

    va_start(ap, n);

    for(i = 0; n > i;) {
        types[i++] = va_arg(ap, Sk_TokenType);
    }

    token = Sk_Scanner_next(scanner);

    while(token.type != SK_EOF) {
        for(i = 0; n > i; i++) {
            if(types[i] == token.type) {
                return;
            }
        }
        token = Sk_Scanner_next(scanner);
    }
}

static void
Sk_Scanner_set_string_token(Sk_Scanner* scanner)
{
    int          c;
    size_t       len;
    Sk_Token*    token    = &scanner->token;
    Sk_CharIter* iterator = &scanner->iter;

    token->type       = SK_STRING;
    token->lexeme.ptr = Sk_CharIter_next_address(iterator);
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

static void
Sk_Scanner_set_bool_or_null_token(Sk_Scanner* scanner, char ch)
{
    char*        start        = Sk_CharIter_next_address(&scanner->iter) - 1;
    Sk_CharIter* iter         = &scanner->iter;
    scanner->token.type       = SK_INVALID;
    scanner->token.lexeme.ptr = start;
    scanner->token.lexeme.len = 0;

    switch(ch) {
        case 't':
            if(in_bounds(iter, start + 3) && strncmp(start, "true", 4) == 0) {
                scanner->token.type = SK_TRUE;
                Sk_CharIter_advance(iter, 3);
#ifdef SK_DBUG
                assert(*(Sk_CharIter_next_address(iter) - 1) == 'e');
#endif
                scanner->token.lexeme.len = 4;
            }
            break;
        case 'f':
            if(in_bounds(iter, start + 4) && strncmp(start, "false", 5) == 0) {
                scanner->token.type = SK_FALSE;
                Sk_CharIter_advance(iter, 4);
#ifdef SK_DBUG
                assert(*(Sk_CharIter_next_address(iter) - 1) == 'e');
#endif
                scanner->token.lexeme.len = 5;
            }
            break;
        case 'n':
            if(in_bounds(iter, start + 3) && strncmp(start, "null", 4) == 0) {
                scanner->token.type = SK_NULL;
                Sk_CharIter_advance(iter, 3);
#ifdef SK_DBUG
                assert(*(Sk_CharIter_next_address(iter) - 1) == 'l');
#endif
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
Sk_Token_new(Sk_TokenType type, char* start, size_t len)
{
    return (Sk_Token) {
        .type   = type,
        .lexeme = Sk_Slice_new(start, len),
    };
}

void
Sk_Token_print(Sk_Token token)
{
    switch(token.type) {
        case SK_INVALID:
            printf("SK_INVALID: ");
            break;
        case SK_LCURLY:
            printf("SK_LCURLY: ");
            break;
        case SK_RCURLY:
            printf("SK_RCURLY: ");
            break;
        case SK_LBRACK:
            printf("SK_LBRACK: ");
            break;
        case SK_RBRACK:
            printf("SK_RBRACK: ");
            break;
        case SK_WS:
            printf("SK_WS: ");
            break;
        case SK_NL:
            printf("SK_NL: ");
            break;
        case SK_STRING:
            printf("SK_STRING: ");
            break;
        case SK_EOF:
            printf("SK_EOF: ");
            break;
        case SK_DIGIT:
            printf("SK_DIGIT: ");
            break;
        case SK_ZERO:
            printf("SK_ZERO: ");
            break;
        case SK_DOT:
            printf("SK_DOT: ");
            break;
        case SK_HYPHEN:
            printf("SK_HYPHEN: ");
            break;
        case SK_EXP:
            printf("SK_EXP: ");
            break;
        case SK_PLUS:
            printf("SK_PLUS: ");
            break;
        case SK_TRUE:
            printf("SK_TRUE: ");
            break;
        case SK_FALSE:
            printf("SK_FALSE: ");
            break;
        case SK_NULL:
            printf("SK_NULL: ");
            break;
        case SK_COMMA:
            printf("SK_COMMA: ");
            break;
        case SK_COLON:
            printf("SK_COLON: ");
            break;
    }

    printf("lexeme = %.*s, len = %lu\n",
           (int) token.lexeme.len,
           token.lexeme.ptr,
           token.lexeme.len);
}

Sk_Token
Sk_Scanner_next(Sk_Scanner* scanner)
{
    Sk_Token* token = &scanner->token;
    int       c;

    char* ch = Sk_CharIter_next_address(&scanner->iter);
    switch((c = Sk_CharIter_next(&scanner->iter))) {
        case '{':
            *token = Sk_Token_new(SK_LCURLY, ch, 1);
            break;
        case '}':
            *token = Sk_Token_new(SK_RCURLY, ch, 1);
            break;
        case '[':
            *token = Sk_Token_new(SK_RBRACK, ch, 1);
            break;
        case ']':
            *token = Sk_Token_new(SK_LBRACK, ch, 1);
            break;
        case ' ':
        case '\t':
            *token = Sk_Token_new(SK_WS, NULL, 0);
            break;
        case '\n':
            *token = Sk_Token_new(SK_NL, NULL, 0);
            break;
        case '"':
            Sk_Scanner_set_string_token(scanner);
            break;
        case '.':
            *token = Sk_Token_new(SK_DOT, ch, 1);
            break;
        case '-':
            *token = Sk_Token_new(SK_HYPHEN, ch, 1);
            break;
        case '+':
            *token = Sk_Token_new(SK_PLUS, ch, 1);
            break;
        case ',':
            *token = Sk_Token_new(SK_COMMA, ch, 1);
            break;
        case ':':
            *token = Sk_Token_new(SK_COLON, ch, 1);
            break;
        case 't':
            Sk_Scanner_set_bool_or_null_token(scanner, 't');
            break;
        case 'f':
            Sk_Scanner_set_bool_or_null_token(scanner, 'f');
            break;
        case 'n':
            Sk_Scanner_set_bool_or_null_token(scanner, 'n');
            break;
        case 'e':
        case 'E':
            *token = Sk_Token_new(SK_EXP, ch, 1);
            break;
        case EOF:
            *token = Sk_Token_new(SK_EOF, NULL, 0);
            break;
        case '0':
            *token = Sk_Token_new(SK_ZERO, ch, 1);
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
            *token = Sk_Token_new(SK_DIGIT, ch, 1);
            break;
        default:
            *token = Sk_Token_new(SK_INVALID, NULL, 0);
            break;
    }

    return scanner->token;
}
