#include "../src/scanner.h"
#include <criterion/criterion.h>
#include <fcntl.h>
#include <unistd.h>

char        buf[BUFSIZ];
int         fd;
int         n;
Sk_Scanner* scanner = NULL;

void
setup(void)
{
    printf("\n\n\n");
    if((fd = open("test.json", 'r')) == -1) {
        fprintf(stderr, "Error opening test.json\n");
        return;
    }

    n = read(fd, buf, BUFSIZ);
}

void
teardown(void)
{
    close(fd);
    free(scanner);
}

Test(SkScanner, ParseTokens, .init = setup, .fini = teardown)
{
    scanner = Sk_Scanner_new(buf, n);
    cr_assert(scanner != NULL);

    Sk_Token token = Sk_Scanner_next(scanner);
    cr_assert(SK_LCURLY == token.type);
    cr_assert(SK_LCURLY == Sk_Scanner_peek(scanner).type);
    cr_assert(token.lexeme.len == 0);
    cr_assert(token.lexeme.ptr == NULL);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_NL == token.type);
    cr_assert(SK_NL == Sk_Scanner_peek(scanner).type);
    cr_assert(token.lexeme.len == 0);
    cr_assert(token.lexeme.ptr == NULL);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_WS == token.type);
    cr_assert(SK_WS == Sk_Scanner_peek(scanner).type);
    cr_assert(token.lexeme.len == 0);
    cr_assert(token.lexeme.ptr == NULL);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_STRING == token.type);
    cr_assert(SK_STRING == Sk_Scanner_peek(scanner).type);
    cr_assert(*token.lexeme.ptr == 'g');
    cr_assert(*(token.lexeme.ptr + 7) == 'y');
    cr_assert(token.lexeme.len == 8);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_COLON == token.type);
    cr_assert(SK_COLON == Sk_Scanner_peek(scanner).type);
    cr_assert(token.lexeme.ptr == NULL);
    cr_assert(token.lexeme.len == 0);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_WS == token.type);
    cr_assert(SK_WS == Sk_Scanner_peek(scanner).type);
    cr_assert(token.lexeme.ptr == NULL);
    cr_assert(token.lexeme.len == 0);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_LCURLY == token.type);
    cr_assert(SK_LCURLY == Sk_Scanner_peek(scanner).type);
    cr_assert(token.lexeme.ptr == NULL);
    cr_assert(token.lexeme.len == 0);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_NL == token.type);
    cr_assert(SK_NL == Sk_Scanner_peek(scanner).type);
    cr_assert(token.lexeme.ptr == NULL);
    cr_assert(token.lexeme.len == 0);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_WS == token.type);
    cr_assert(SK_WS == Sk_Scanner_peek(scanner).type);
    cr_assert(token.lexeme.ptr == NULL);
    cr_assert(token.lexeme.len == 0);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_WS == token.type);
    cr_assert(SK_WS == Sk_Scanner_peek(scanner).type);
    cr_assert(token.lexeme.ptr == NULL);
    cr_assert(token.lexeme.len == 0);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_STRING == token.type);
    cr_assert(SK_STRING == Sk_Scanner_peek(scanner).type);
    cr_assert(*token.lexeme.ptr == 't');
    cr_assert(*(token.lexeme.ptr + 4) == 'e');
    cr_assert(token.lexeme.len == 5);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_COLON == token.type);
    cr_assert(SK_COLON == Sk_Scanner_peek(scanner).type);
    cr_assert(token.lexeme.ptr == NULL);
    cr_assert(token.lexeme.len == 0);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_WS == token.type);
    cr_assert(SK_WS == Sk_Scanner_peek(scanner).type);
    cr_assert(token.lexeme.ptr == NULL);
    cr_assert(token.lexeme.len == 0);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_STRING == token.type);
    cr_assert(SK_STRING == Sk_Scanner_peek(scanner).type);
    cr_assert(*token.lexeme.ptr == 'e');
    cr_assert(*(token.lexeme.ptr + token.lexeme.len - 1) == 'y');
    cr_assert(token.lexeme.len == 16);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_COMMA == token.type);
    cr_assert(SK_COMMA == Sk_Scanner_peek(scanner).type);
    cr_assert(token.lexeme.ptr == NULL);
    cr_assert(token.lexeme.len == 0);

    Sk_Scanner_skip(scanner, 2, SK_WS, SK_NL);
    token = Sk_Scanner_peek(scanner);
    cr_assert(SK_STRING == token.type);
    cr_assert(SK_STRING == Sk_Scanner_peek(scanner).type);
    cr_assert(*token.lexeme.ptr == 'G');
    cr_assert(*(token.lexeme.ptr + token.lexeme.len - 1) == 'v');
    cr_assert(token.lexeme.len == 8);

    Sk_Scanner_skip_until(scanner, 1, SK_DIGIT);
    token = Sk_Scanner_peek(scanner);
    cr_assert(SK_DIGIT == token.type);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_DIGIT == token.type);
    cr_assert(SK_DIGIT == Sk_Scanner_peek(scanner).type);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_DIGIT == token.type);
    cr_assert(SK_DIGIT == Sk_Scanner_peek(scanner).type);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_COMMA == token.type);
    cr_assert(SK_COMMA == Sk_Scanner_peek(scanner).type);

    Sk_Scanner_skip_until(scanner, 1, SK_STRING);
    token = Sk_Scanner_peek(scanner);
    cr_assert(SK_STRING == token.type);
    cr_assert(SK_WS == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_COLON == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_WS == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_HYPHEN == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_DIGIT == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_DIGIT == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_DOT == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_DIGIT == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_DIGIT == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_DIGIT == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_EXP == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_PLUS == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_ZERO == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_ZERO == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_ZERO == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_DIGIT == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_DIGIT == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_COMMA == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_NL == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_WS == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_WS == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_WS == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_WS == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_WS == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_WS == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_WS == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_WS == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_WS == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_WS == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_WS == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_WS == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_STRING == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(strncmp(token.lexeme.ptr, "numnum", token.lexeme.len) == 0);

    Sk_Scanner_skip_until(scanner, 1, SK_TRUE);
    token = Sk_Scanner_peek(scanner);
    cr_assert(SK_TRUE == token.type);
    cr_assert(strncmp(token.lexeme.ptr, "true", token.lexeme.len) == 0);
    cr_assert(SK_COMMA == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_WS == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_STRING == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(strncmp(token.lexeme.ptr, "asd", token.lexeme.len) == 0);
    cr_assert(SK_WS == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_COLON == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_WS == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_FALSE == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(strncmp(token.lexeme.ptr, "false", token.lexeme.len) == 0);
    cr_assert(SK_COMMA == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_WS == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_STRING == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(strncmp(token.lexeme.ptr, "nulll", token.lexeme.len) == 0);
    cr_assert(SK_WS == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_COLON == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_WS == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_NULL == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(strncmp(token.lexeme.ptr, "null", token.lexeme.len) == 0);
    cr_assert(SK_COMMA == (token = Sk_Scanner_next(scanner)).type);
    Sk_Scanner_skip_until(scanner, 1, SK_ZERO);
    token = Sk_Scanner_peek(scanner);
    cr_assert(SK_ZERO == token.type);
    cr_assert(SK_DIGIT == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_DOT == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_DIGIT == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_DIGIT == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_COMMA == (token = Sk_Scanner_next(scanner)).type);
    cr_assert(SK_NL == (token = Sk_Scanner_next(scanner)).type);
}
