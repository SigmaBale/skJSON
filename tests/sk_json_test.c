#include "../src/parser.h"
#include <criterion/criterion.h>
#include <criterion/internal/assert.h>
#include <fcntl.h>
#include <unistd.h>

Sk_Scanner* scanner;
char        buf[BUFSIZ];
int         fd;
int         n;

void
json_setup(void)
{
    printf("\n\n\n");
    if((fd = open("test.json", 'r')) == -1) {
        fprintf(stderr, "Error opening test.json\n");
        return;
    }

    n = read(fd, buf, BUFSIZ);

    scanner = Sk_Scanner_new(buf, n);
}

void
json_teardown(void)
{
    free(scanner);
    close(fd);
}

TestSuite(SkJson, .init = json_setup, .fini = json_teardown);

Test(SkJson, ParseTokens)
{
    cr_assert(scanner != NULL);

    Sk_Token token = Sk_Scanner_next(scanner);
    cr_assert(SK_LCURLY == token.type);
    cr_assert(SK_LCURLY == Sk_Scanner_peek(scanner).type);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_NL == token.type);
    cr_assert(SK_NL == Sk_Scanner_peek(scanner).type);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_WS == token.type);
    cr_assert(SK_WS == Sk_Scanner_peek(scanner).type);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_STRING == token.type);
    cr_assert(SK_STRING == Sk_Scanner_peek(scanner).type);
    cr_assert(*token.lexeme.ptr == 'g');
    cr_assert(*(token.lexeme.ptr + 7) == 'y');
    cr_assert(token.lexeme.len == 8);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_COLON == token.type);
    cr_assert(SK_COLON == Sk_Scanner_peek(scanner).type);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_WS == token.type);
    cr_assert(SK_WS == Sk_Scanner_peek(scanner).type);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_LCURLY == token.type);
    cr_assert(SK_LCURLY == Sk_Scanner_peek(scanner).type);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_NL == token.type);
    cr_assert(SK_NL == Sk_Scanner_peek(scanner).type);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_WS == token.type);
    cr_assert(SK_WS == Sk_Scanner_peek(scanner).type);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_WS == token.type);
    cr_assert(SK_WS == Sk_Scanner_peek(scanner).type);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_STRING == token.type);
    cr_assert(SK_STRING == Sk_Scanner_peek(scanner).type);
    cr_assert(*token.lexeme.ptr == 't');
    cr_assert(*(token.lexeme.ptr + 4) == 'e');
    cr_assert(token.lexeme.len == 5);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_COLON == token.type);
    cr_assert(SK_COLON == Sk_Scanner_peek(scanner).type);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_WS == token.type);
    cr_assert(SK_WS == Sk_Scanner_peek(scanner).type);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_STRING == token.type);
    cr_assert(SK_STRING == Sk_Scanner_peek(scanner).type);
    cr_assert(*token.lexeme.ptr == 'e');
    cr_assert(*(token.lexeme.ptr + token.lexeme.len - 1) == 'y');
    cr_assert(token.lexeme.len == 16);

    token = Sk_Scanner_next(scanner);
    cr_assert(SK_COMMA == token.type);
    cr_assert(SK_COMMA == Sk_Scanner_peek(scanner).type);

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

    Sk_Scanner_skip_until(scanner, 1, SK_EOF);
    cr_assert(SK_EOF == Sk_Scanner_peek(scanner).type);
    cr_assert(SK_EOF == (token = Sk_Scanner_next(scanner)).type);
}

Test(SkJson, ParseObjects)
{
    cr_assert(scanner != NULL);

    Sk_Token token = Sk_Scanner_next(scanner);
    cr_assert(SK_LCURLY == token.type);

    Sk_Scanner_skip(scanner, 2, SK_WS, SK_NL);
    cr_assert(SK_STRING == (token = Sk_Scanner_peek(scanner)).type);

    Sk_JsonNode* str_node = Sk_parse_json_string(scanner);
    cr_assert(str_node != NULL);
    cr_assert_eq(strcmp(str_node->data->j_string, "glossary"), 0);
    cr_assert(str_node->type == SK_STRING_NODE);
    Sk_JsonNode_drop(str_node);

    Sk_Scanner_skip_until(scanner, 1, SK_DIGIT);
    token = Sk_Scanner_peek(scanner);
    cr_assert(SK_DIGIT == token.type);

    Sk_JsonNode* num_node = Sk_parse_json_number(scanner);
    cr_assert(num_node != NULL);
    cr_assert_eq(num_node->data->j_int, 152);
    cr_assert_eq(num_node->type, SK_INT_NODE);
    Sk_JsonNode_drop(num_node);

    Sk_Scanner_skip_until(scanner, 1, SK_HYPHEN);
    token = Sk_Scanner_peek(scanner);
    cr_assert_eq(token.type, SK_HYPHEN);

    Sk_JsonNode* dbl_node = Sk_parse_json_number(scanner);
    cr_assert(dbl_node->type == SK_DOUBLE_NODE);
    cr_assert(dbl_node->data->j_double == -12.523e15);
    Sk_JsonNode_drop(dbl_node);

    Sk_Scanner_skip_until(scanner, 1, SK_NULL);
    cr_assert(SK_NULL == Sk_Scanner_peek(scanner).type);
    Sk_Scanner_skip_until(scanner, 1, SK_ZERO);
    cr_assert(SK_ZERO == (token = Sk_Scanner_peek(scanner)).type);

    Sk_JsonNode* err_node = Sk_parse_json_number(scanner);
    cr_assert(SK_ERROR_NODE == err_node->type);
    Sk_JsonNode_drop(err_node);
}
