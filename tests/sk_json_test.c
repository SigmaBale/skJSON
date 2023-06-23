// clang-format off
#include <criterion/criterion.h>
#include <criterion/internal/assert.h>
#include "../src/skparser.h"
#include <fcntl.h>
#include <unistd.h>
// clang-format on

skScanner* scanner;
char       buf[BUFSIZ];
int        fd;
int        n;

void
json_setup(void)
{
    printf("\n\n\n");
    if((fd = open("test.json", 'r')) == -1) {
        fprintf(stderr, "Error opening test.json\n");
        return;
    }

    n = read(fd, buf, BUFSIZ);

    scanner = skScanner_new(buf, n);
}

void
json_teardown(void)
{
    free(scanner);
    close(fd);
}

// TestSuite(SkJson, .init = json_setup, .fini = json_teardown);
//
// Test(SkJson, ParseTokens)
//{
//     cr_assert(scanner != NULL);
//
//     skToken token = skScanner_next(scanner);
//     cr_assert(SK_LCURLY == token.type);
//     cr_assert(SK_LCURLY == skScanner_peek(scanner).type);
//
//     token = skScanner_next(scanner);
//     cr_assert(SK_NL == token.type);
//     cr_assert(SK_NL == skScanner_peek(scanner).type);
//
//     token = skScanner_next(scanner);
//     cr_assert(SK_WS == token.type);
//     cr_assert(SK_WS == skScanner_peek(scanner).type);
//
//     token = skScanner_next(scanner);
//     cr_assert(SK_STRING == token.type);
//     cr_assert(SK_STRING == skScanner_peek(scanner).type);
//     cr_assert(*token.lexeme.ptr == 'g');
//     cr_assert(*(token.lexeme.ptr + 7) == 'y');
//     cr_assert(token.lexeme.len == 8);
//
//     token = skScanner_next(scanner);
//     cr_assert(SK_COLON == token.type);
//     cr_assert(SK_COLON == skScanner_peek(scanner).type);
//
//     token = skScanner_next(scanner);
//     cr_assert(SK_WS == token.type);
//     cr_assert(SK_WS == skScanner_peek(scanner).type);
//
//     token = skScanner_next(scanner);
//     cr_assert(SK_LCURLY == token.type);
//     cr_assert(SK_LCURLY == skScanner_peek(scanner).type);
//
//     token = skScanner_next(scanner);
//     cr_assert(SK_NL == token.type);
//     cr_assert(SK_NL == skScanner_peek(scanner).type);
//
//     token = skScanner_next(scanner);
//     cr_assert(SK_WS == token.type);
//     cr_assert(SK_WS == skScanner_peek(scanner).type);
//
//     token = skScanner_next(scanner);
//     cr_assert(SK_WS == token.type);
//     cr_assert(SK_WS == skScanner_peek(scanner).type);
//
//     token = skScanner_next(scanner);
//     cr_assert(SK_STRING == token.type);
//     cr_assert(SK_STRING == skScanner_peek(scanner).type);
//     cr_assert(*token.lexeme.ptr == 't');
//     cr_assert(*(token.lexeme.ptr + 4) == 'e');
//     cr_assert(token.lexeme.len == 5);
//
//     token = skScanner_next(scanner);
//     cr_assert(SK_COLON == token.type);
//     cr_assert(SK_COLON == skScanner_peek(scanner).type);
//
//     token = skScanner_next(scanner);
//     cr_assert(SK_WS == token.type);
//     cr_assert(SK_WS == skScanner_peek(scanner).type);
//
//     token = skScanner_next(scanner);
//     cr_assert(SK_STRING == token.type);
//     cr_assert(SK_STRING == skScanner_peek(scanner).type);
//     cr_assert(*token.lexeme.ptr == 'e');
//     cr_assert(*(token.lexeme.ptr + token.lexeme.len - 1) == 'y');
//     cr_assert(token.lexeme.len == 16);
//
//     token = skScanner_next(scanner);
//     cr_assert(SK_COMMA == token.type);
//     cr_assert(SK_COMMA == skScanner_peek(scanner).type);
//
//     skScanner_skip(scanner, 2, SK_WS, SK_NL);
//     token = skScanner_peek(scanner);
//     cr_assert(SK_STRING == token.type);
//     cr_assert(SK_STRING == skScanner_peek(scanner).type);
//     cr_assert(*token.lexeme.ptr == 'G');
//     cr_assert(*(token.lexeme.ptr + token.lexeme.len - 1) == 'v');
//     cr_assert(token.lexeme.len == 8);
//
//     skScanner_skip_until(scanner, 1, SK_DIGIT);
//     token = skScanner_peek(scanner);
//     cr_assert(SK_DIGIT == token.type);
//
//     token = skScanner_next(scanner);
//     cr_assert(SK_DIGIT == token.type);
//     cr_assert(SK_DIGIT == skScanner_peek(scanner).type);
//
//     token = skScanner_next(scanner);
//     cr_assert(SK_DIGIT == token.type);
//     cr_assert(SK_DIGIT == skScanner_peek(scanner).type);
//
//     token = skScanner_next(scanner);
//     cr_assert(SK_COMMA == token.type);
//     cr_assert(SK_COMMA == skScanner_peek(scanner).type);
//
//     skScanner_skip_until(scanner, 1, SK_STRING);
//     token = skScanner_peek(scanner);
//     cr_assert(SK_STRING == token.type);
//     cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_COLON == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_HYPHEN == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_DIGIT == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_DIGIT == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_DOT == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_DIGIT == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_DIGIT == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_DIGIT == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_EXP == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_PLUS == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_ZERO == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_ZERO == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_ZERO == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_DIGIT == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_DIGIT == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_COMMA == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_NL == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_STRING == (token = skScanner_next(scanner)).type);
//     cr_assert(strncmp(token.lexeme.ptr, "numnum", token.lexeme.len) == 0);
//
//     skScanner_skip_until(scanner, 1, SK_TRUE);
//     token = skScanner_peek(scanner);
//     cr_assert(SK_TRUE == token.type);
//     cr_assert(strncmp(token.lexeme.ptr, "true", token.lexeme.len) == 0);
//     cr_assert(SK_COMMA == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_STRING == (token = skScanner_next(scanner)).type);
//     cr_assert(strncmp(token.lexeme.ptr, "asd", token.lexeme.len) == 0);
//     cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_COLON == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_FALSE == (token = skScanner_next(scanner)).type);
//     cr_assert(strncmp(token.lexeme.ptr, "false", token.lexeme.len) == 0);
//     cr_assert(SK_COMMA == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_STRING == (token = skScanner_next(scanner)).type);
//     cr_assert(strncmp(token.lexeme.ptr, "nulll", token.lexeme.len) == 0);
//     cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_COLON == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_NULL == (token = skScanner_next(scanner)).type);
//     cr_assert(strncmp(token.lexeme.ptr, "null", token.lexeme.len) == 0);
//     cr_assert(SK_COMMA == (token = skScanner_next(scanner)).type);
//     skScanner_skip_until(scanner, 1, SK_ZERO);
//     token = skScanner_peek(scanner);
//     cr_assert(SK_ZERO == token.type);
//     cr_assert(SK_DIGIT == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_DOT == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_DIGIT == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_DIGIT == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_COMMA == (token = skScanner_next(scanner)).type);
//     cr_assert(SK_NL == (token = skScanner_next(scanner)).type);
//
//     skScanner_skip_until(scanner, 1, SK_EOF);
//     cr_assert(SK_EOF == skScanner_peek(scanner).type);
//     cr_assert(SK_EOF == (token = skScanner_next(scanner)).type);
// }

// Test(SkJson, ParsePrimitives)
//{
//     cr_assert(scanner != NULL);
//
//     skToken token = skScanner_next(scanner);
//     cr_assert(SK_LCURLY == token.type);
//
//     skScanner_skip(scanner, 2, SK_WS, SK_NL);
//     cr_assert(SK_STRING == (token = skScanner_peek(scanner)).type);
//
//     skJsonNode* str_node = skparse_json_string(scanner);
//     cr_assert(str_node != NULL);
//     cr_assert_eq(strcmp(str_node->data->j_string, "glossary"), 0);
//     cr_assert(str_node->type == SK_STRING_NODE);
//     skJsonNode_drop(str_node);
//
//     skScanner_skip_until(scanner, 1, SK_DIGIT);
//     token = skScanner_peek(scanner);
//     cr_assert(SK_DIGIT == token.type);
//
//     skJsonNode* num_node = skparse_json_number(scanner);
//     cr_assert(num_node != NULL);
//     cr_assert_eq(num_node->data->j_int, 152);
//     cr_assert_eq(num_node->type, SK_INT_NODE);
//     skJsonNode_drop(num_node);
//
//     skScanner_skip_until(scanner, 1, SK_HYPHEN);
//     token = skScanner_peek(scanner);
//     cr_assert_eq(token.type, SK_HYPHEN);
//
//     skJsonNode* dbl_node = skparse_json_number(scanner);
//     cr_assert(dbl_node->type == SK_DOUBLE_NODE);
//     cr_assert(dbl_node->data->j_double == -12.523e15);
//     skJsonNode_drop(dbl_node);
//
//     skScanner_skip_until(scanner, 1, SK_TRUE);
//     skJsonNode* bool_node = skparse_json_bool(scanner);
//     cr_assert(bool_node != NULL);
//     cr_assert(bool_node->type == SK_BOOL_NODE);
//     cr_assert(bool_node->data->j_boolean == true);
//     skJsonNode_drop(bool_node);
//
//     skScanner_skip_until(scanner, 1, SK_FALSE);
//     bool_node = skparse_json_bool(scanner);
//     cr_assert(bool_node != NULL);
//     cr_assert(bool_node->type == SK_BOOL_NODE);
//     cr_assert(bool_node->data->j_boolean == false);
//     skJsonNode_drop(bool_node);
//
//     skScanner_skip_until(scanner, 1, SK_NULL);
//     skJsonNode* null_node = skparse_json_null();
//     cr_assert(null_node != NULL);
//     cr_assert(null_node->type == SK_NULL_NODE);
//     cr_assert(null_node->data->j_null == NULL);
//     skJsonNode_drop(null_node);
//
//     skScanner_skip_until(scanner, 1, SK_ZERO);
//     cr_assert(SK_ZERO == (token = skScanner_peek(scanner)).type);
//
//     skJsonNode* err_node = skparse_json_number(scanner);
//     cr_assert(SK_ERROR_NODE == err_node->type);
//     cr_assert_eq(strcmp(err_node->data->j_err, "failed to parse Json
//     Number"), 0); skJsonNode_drop(err_node);
// }

void
json_setup_complex(void)
{
    printf("\n\n\n");
    if((fd = open("complex.json", 'r')) == -1) {
        fprintf(stderr, "Error opening complex.json\n");
        return;
    }

    n = read(fd, buf, BUFSIZ);

    scanner = skScanner_new(buf, n);
}

TestSuite(
    SkJsonComplexTypes,
    .init = json_setup_complex,
    .fini = json_teardown);

Test(SkJsonComplexTypes, ParseObjects)
{
    skToken token = skScanner_next(scanner);
    cr_assert(token.type == SK_LCURLY);
    cr_assert(skScanner_next(scanner).type == SK_NL);
    cr_assert(skScanner_next(scanner).type == SK_WS);
    cr_assert(skScanner_next(scanner).type == SK_WS);
    cr_assert(skScanner_next(scanner).type == SK_WS);
    cr_assert(skScanner_next(scanner).type == SK_WS);
    cr_assert(skScanner_next(scanner).type == SK_STRING);
    cr_assert(skScanner_next(scanner).type == SK_WS);
    cr_assert(skScanner_next(scanner).type == SK_COLON);
    cr_assert(skScanner_next(scanner).type == SK_WS);
    cr_assert(skScanner_next(scanner).type == SK_LBRACK);

    skJsonNode* arr_node = skparse_json_array(scanner, NULL);
    cr_assert(arr_node != NULL);
    cr_assert(arr_node->type == SK_ARRAY_NODE);
    cr_assert(arr_node->data.j_array->len == 7);
    skVec*      nodes = arr_node->data.j_array;
    skJsonNode* temp;
    cr_assert(
        (temp = ((skJsonNode*) skVec_index(nodes, 0)))->type == SK_STRING_NODE);
    printf("String node ok, its parent -> ");
    print_node(temp->parent);
    cr_assert(
        (temp = ((skJsonNode*) skVec_index(nodes, 1)))->type == SK_STRING_NODE);
    printf("String node ok, its parent -> ");
    print_node(temp->parent);
    cr_assert(
        (temp = ((skJsonNode*) skVec_index(nodes, 2)))->type == SK_INT_NODE);
    printf("Integer node ok, its parent -> ");
    print_node(temp->parent);
    cr_assert(
        (temp = ((skJsonNode*) skVec_index(nodes, 3)))->type == SK_DOUBLE_NODE);
    printf("Double node ok, its parent -> ");
    print_node(temp->parent);
    cr_assert(
        (temp = ((skJsonNode*) skVec_index(nodes, 4)))->type == SK_BOOL_NODE);
    printf("bool node ok, its parent -> ");
    print_node(temp->parent);
    cr_assert(
        (temp = ((skJsonNode*) skVec_index(nodes, 5)))->type == SK_BOOL_NODE);
    printf("bool node ok, its parent -> ");
    print_node(temp->parent);
    cr_assert(
        (temp = ((skJsonNode*) skVec_index(nodes, 6)))->type == SK_NULL_NODE);
    printf("null node ok, its parent -> ");
    print_node(temp->parent);
    printf("All passed, now fix drop\n");
    skJsonNode_drop(arr_node);
}
