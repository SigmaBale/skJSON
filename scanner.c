#include "node.h"
#include "scanner.h"
#include "sk_vec.h"
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFF_READ_ERR fprintf(stderr, "errored trying to read into buffer\n")
#define OFFSET_OOB    fprintf(stderr, "offset is out of bounds\n")

Sk_Scanner*
Sk_Scanner_new(int fd, size_t bufsize)
{
    if(bufsize == 0) {
        bufsize = BUFSIZ;
    }

    char* buffer = malloc(bufsize);

    if(buffer == NULL) {
        PRINT_OOM_ERR;
        return NULL;
    }

    Sk_Scanner* scanner = malloc(sizeof(Sk_Scanner));

    if(scanner == NULL) {
        PRINT_OOM_ERR;
        free(buffer);
        return NULL;
    }

    scanner->iter   = Sk_CharIter_new(buffer, bufsize - 1);
    scanner->token  = (Sk_Token) { 0 };
    scanner->fd     = fd;
    scanner->buffer = buffer;
    scanner->ptr    = buffer;

    return scanner;
}

static int
_Sk_Scanner_refill_buf(Sk_Scanner* scanner)
{

    int n;
    /// Read more data into scanner buffer
    n = read(scanner->fd, scanner->buffer, scanner->bufsize);

    scanner->bufsize = (n == -1) ? 0 : n;

    return n;
}

static inline void
_Sk_Scanner_refill_and_handle(Sk_Scanner* scanner)
{
    int ret;
    if((ret = _Sk_Scanner_refill_buf(scanner)) == 0) {
        /// We reached end of the file, return EOF token
        scanner->token.type = SK_EOF;
    } else if(ret == -1) {
        /// If error occured, exit with failure
        BUFF_READ_ERR;
        exit(EXIT_FAILURE);
    } else {
        /// If buffer is refilled get the next token
        scanner->token = Sk_Scanner_next(scanner);
    }
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
        /// Check if we hit end of iterator and try to refill buffer
        if(c == EOF && (c = _Sk_Scanner_refill_buf(scanner)) == -1) {
            BUFF_READ_ERR;
            /// Reading into buff failed, exit
            exit(EXIT_FAILURE);
        } else if(c == 0) {
            /// We reached end of file, but string is invalid
            scanner->token.type = SK_INVALID;
            break;
        }
    }

    /// String is properly enclosed
    token->lexeme.len = len;
}

// TODO: Implement these
void static _Sk_Scanner_set_true_token(Sk_Scanner* scanner);
void static _Sk_Scanner_set_false_token(Sk_Scanner* scanner);
void static _Sk_Scanner_set_null_token(Sk_Scanner* scanner);

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
            scanner->token.type = SK_SEMICOLON;
            break;
        case 't':
            _Sk_Scanner_set_true_token(scanner);
            break;
        case 'f':
            _Sk_Scanner_set_false_token(scanner);
            break;
        case 'n':
            _Sk_Scanner_set_null_token(scanner);
            break;
        case 'e':
        case 'E':
            scanner->token.type = SK_EXP;
            break;
        case EOF:
            _Sk_Scanner_refill_and_handle(scanner);
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

    Sk_CharIter_next(&scanner->iter);
    return scanner->token;
}
