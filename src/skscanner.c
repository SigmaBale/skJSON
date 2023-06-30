#ifdef SK_DBUG
#include <assert.h>
#endif
/* clang-format off */
#include <stdbool.h>
#include "skerror.h"
#include "skutils.h"
#include "skscanner.h"
#include "skvec.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#define in_iter_bounds(iter, bound) ((iter)->end >= (bound))

skScanner*
skScanner_new(void* buffer, size_t bufsize)
{
    skScanner* scanner;

    if(is_null(buffer) || bufsize == 0) {
        return NULL;
    }

    scanner = malloc(sizeof(skScanner));
    if(is_null(scanner)) {
        THROW_ERR(OutOfMemory);
        return NULL;
    }

    /* Leave token field as random garbo */
    scanner->iter = skCharIter_new(buffer, bufsize - 1);

    return scanner;
}

skToken
skScanner_peek(const skScanner* scanner)
{
    return scanner->token;
}

void
skScanner_skip(skScanner* scanner, size_t n, ...)
{
    skToken      token;
    /* Variable sized arrays are not supported by C90 */
    skTokenType  types[3];
    va_list      ap;
    unsigned int i;

    va_start(ap, n);

    for(i = 0; n > i; i++) {
        types[i] = va_arg(ap, skTokenType);
    }

    token = skScanner_peek(scanner);
    for(i = 0; n > i; i++) {
        if(types[i] == token.type) {
            if((token = skScanner_next(scanner)).type == SK_EOF) {
                return;
            }
            i = -1;
        }
    }

    va_end(ap);
}

void
skScanner_skip_until(skScanner* scanner, size_t n, ...)
{
    skToken      token;
    /* Variable sized arrays are not supported by C90 */
    skTokenType  types[3];
    va_list      ap;
    unsigned int i;

    if(n == 0) {
        return;
    }

    va_start(ap, n);

    for(i = 0; n > i;) {
        types[i++] = va_arg(ap, skTokenType);
    }

    token = skScanner_peek(scanner);
    while(token.type != SK_EOF) {
        for(i = 0; n > i; i++) {
            if(types[i] == token.type) {
                return;
            }
        }
        token = skScanner_next(scanner);
    }
}

static void
skScanner_set_string_token(skScanner* scanner)
{
    int         c;
    size_t      len;
    skToken*    token;
    skCharIter* iterator;

    token    = &scanner->token;
    iterator = &scanner->iter;

    token->type       = SK_STRING;
    token->lexeme.ptr = skCharIter_next_address(iterator);
    token->lexeme.len = 0;

    iterator->state.in_jstring = true;
    /* Advance iterator until we hit closing quotes */
    for(len = 0; (c = skCharIter_next(iterator)) != '"'; len++) {
        if(c == EOF) {
            /* We reached end of file and string is invalid */
            scanner->token.type = SK_INVALID;
            break;
        }
    }
    iterator->state.in_jstring = false;

    /* String is properly enclosed */
    token->lexeme.len = len;
}

static void
skScanner_set_bool_or_null_token(skScanner* scanner, char ch)
{
    char*       start;
    skCharIter* iter;

    start = skCharIter_next_address(&scanner->iter) - 1;
    iter  = &scanner->iter;

    scanner->token.type       = SK_INVALID;
    scanner->token.lexeme.ptr = start;
    scanner->token.lexeme.len = 0;

    switch(ch) {
        case 't':
            if(in_iter_bounds(iter, start + 3) && strncmp(start, "true", 4) == 0) {
                scanner->token.type = SK_TRUE;
                skCharIter_advance(iter, 3);
#ifdef SK_DBUG
                assert(*(skCharIter_next_address(iter) - 1) == 'e');
#endif
                scanner->token.lexeme.len = 4;
            }
            break;
        case 'f':
            if(in_iter_bounds(iter, start + 4) && strncmp(start, "false", 5) == 0) {
                scanner->token.type = SK_FALSE;
                skCharIter_advance(iter, 4);
#ifdef SK_DBUG
                assert(*(skCharIter_next_address(iter) - 1) == 'e');
#endif
                scanner->token.lexeme.len = 5;
            }
            break;
        case 'n':
            if(in_iter_bounds(iter, start + 3) && strncmp(start, "null", 4) == 0) {
                scanner->token.type = SK_NULL;
                skCharIter_advance(iter, 3);
#ifdef SK_DBUG
                assert(*(skCharIter_next_address(iter) - 1) == 'l');
#endif
                scanner->token.lexeme.len = 4;
            }
            break;
        default:
#ifdef SK_DBUG
            assert(false);
#endif
            break;
    }
}

static skToken
skToken_new(skTokenType type, char* start, size_t len)
{
    skToken token;
    token.type   = type;
    token.lexeme = skSlice_new(start, len);
    return token;
}

skToken
skScanner_next(skScanner* scanner)
{
    skToken* token = &scanner->token;
    int      c;
    char*    ch;

    ch = skCharIter_next_address(&scanner->iter);
    switch((c = skCharIter_next(&scanner->iter))) {
        case '{':
            *token = skToken_new(SK_LCURLY, ch, 1);
            break;
        case '}':
            *token = skToken_new(SK_RCURLY, ch, 1);
            break;
        case '[':
            *token = skToken_new(SK_LBRACK, ch, 1);
            break;
        case ']':
            *token = skToken_new(SK_RBRACK, ch, 1);
            break;
        case ' ':
        case '\t':
            *token = skToken_new(SK_WS, ch, 1);
            break;
        case '\n':
            *token = skToken_new(SK_NL, ch, 1);
            break;
        case '"':
            skScanner_set_string_token(scanner);
            break;
        case '.':
            *token = skToken_new(SK_DOT, ch, 1);
            break;
        case '-':
            *token = skToken_new(SK_HYPHEN, ch, 1);
            break;
        case '+':
            *token = skToken_new(SK_PLUS, ch, 1);
            break;
        case ',':
            *token = skToken_new(SK_COMMA, ch, 1);
            break;
        case ':':
            *token = skToken_new(SK_COLON, ch, 1);
            break;
        case 't':
            skScanner_set_bool_or_null_token(scanner, 't');
            break;
        case 'f':
            skScanner_set_bool_or_null_token(scanner, 'f');
            break;
        case 'n':
            skScanner_set_bool_or_null_token(scanner, 'n');
            break;
        case 'e':
        case 'E':
            *token = skToken_new(SK_EXP, ch, 1);
            break;
        case EOF:
            *token = skToken_new(SK_EOF, NULL, 0);
            break;
        case '0':
            *token = skToken_new(SK_ZERO, ch, 1);
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
            *token = skToken_new(SK_DIGIT, ch, 1);
            break;
        default:
            *token = skToken_new(SK_INVALID, ch, 1);
            break;
    }

    return scanner->token;
}
