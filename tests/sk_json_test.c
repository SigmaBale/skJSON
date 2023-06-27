#include "../src/skparser.h"
#include <criterion/criterion.h>
#include <criterion/internal/assert.h>
#include <fcntl.h>
#include <unistd.h>

skJsonString skJsonString_new_internal(skToken token);
skJsonNode*  skparse_json_object(skScanner* scanner, skJsonNode* parent);
skJsonNode*  skparse_json_array(skScanner* scanner, skJsonNode* parent);
skJsonNode*  skparse_json_string(skScanner* scanner, skJsonNode* parent);
skJsonNode*  skparse_json_number(skScanner* scanner, skJsonNode* parent);
skJsonNode*  skparse_json_bool(skScanner* scanner, skJsonNode* parent);
skJsonNode*  skparse_json_null(skScanner* scanner, skJsonNode* parent);

skScanner* scanner;
char       buf[1000000];
int        fd;
int        n;

void
json_setup(void)
{
    printf("\n\n\n");
    if((fd = open("test.json", 'r')) == -1) {
        fprintf(stderr, "Error opening test.json\n");
        exit(EXIT_FAILURE);
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

TestSuite(SkJson, .init = json_setup, .fini = json_teardown);

Test(SkJson, ParseTokens)
{
    cr_assert(scanner != NULL);

    skToken token = skScanner_next(scanner);
    cr_assert(SK_LCURLY == token.type);
    cr_assert(SK_LCURLY == skScanner_peek(scanner).type);

    token = skScanner_next(scanner);
    cr_assert(SK_NL == token.type);
    cr_assert(SK_NL == skScanner_peek(scanner).type);

    token = skScanner_next(scanner);
    cr_assert(SK_WS == token.type);
    cr_assert(SK_WS == skScanner_peek(scanner).type);

    token = skScanner_next(scanner);
    cr_assert(SK_STRING == token.type);
    cr_assert(SK_STRING == skScanner_peek(scanner).type);
    cr_assert(*token.lexeme.ptr == 'g');
    cr_assert(*(token.lexeme.ptr + 7) == 'y');
    cr_assert(token.lexeme.len == 8);

    token = skScanner_next(scanner);
    cr_assert(SK_COLON == token.type);
    cr_assert(SK_COLON == skScanner_peek(scanner).type);

    token = skScanner_next(scanner);
    cr_assert(SK_WS == token.type);
    cr_assert(SK_WS == skScanner_peek(scanner).type);

    token = skScanner_next(scanner);
    cr_assert(SK_LCURLY == token.type);
    cr_assert(SK_LCURLY == skScanner_peek(scanner).type);

    token = skScanner_next(scanner);
    cr_assert(SK_NL == token.type);
    cr_assert(SK_NL == skScanner_peek(scanner).type);

    token = skScanner_next(scanner);
    cr_assert(SK_WS == token.type);
    cr_assert(SK_WS == skScanner_peek(scanner).type);

    token = skScanner_next(scanner);
    cr_assert(SK_WS == token.type);
    cr_assert(SK_WS == skScanner_peek(scanner).type);

    token = skScanner_next(scanner);
    cr_assert(SK_STRING == token.type);
    cr_assert(SK_STRING == skScanner_peek(scanner).type);
    cr_assert(*token.lexeme.ptr == 't');
    cr_assert(*(token.lexeme.ptr + 4) == 'e');
    cr_assert(token.lexeme.len == 5);

    token = skScanner_next(scanner);
    cr_assert(SK_COLON == token.type);
    cr_assert(SK_COLON == skScanner_peek(scanner).type);

    token = skScanner_next(scanner);
    cr_assert(SK_WS == token.type);
    cr_assert(SK_WS == skScanner_peek(scanner).type);

    token = skScanner_next(scanner);
    cr_assert(SK_STRING == token.type);
    cr_assert(SK_STRING == skScanner_peek(scanner).type);
    cr_assert(*token.lexeme.ptr == 'e');
    cr_assert(*(token.lexeme.ptr + token.lexeme.len - 1) == 'y');
    cr_assert(token.lexeme.len == 16);

    token = skScanner_next(scanner);
    cr_assert(SK_COMMA == token.type);
    cr_assert(SK_COMMA == skScanner_peek(scanner).type);
    skScanner_next(scanner);

    skScanner_skip(scanner, 2, SK_WS, SK_NL);
    token = skScanner_peek(scanner);
    cr_assert(SK_STRING == token.type);
    cr_assert(SK_STRING == skScanner_peek(scanner).type);
    cr_assert(*token.lexeme.ptr == 'G');
    cr_assert(*(token.lexeme.ptr + token.lexeme.len - 1) == 'v');
    cr_assert(token.lexeme.len == 8);

    skScanner_skip_until(scanner, 1, SK_DIGIT);
    token = skScanner_peek(scanner);
    cr_assert(SK_DIGIT == token.type);

    token = skScanner_next(scanner);
    cr_assert(SK_DIGIT == token.type);
    cr_assert(SK_DIGIT == skScanner_peek(scanner).type);

    token = skScanner_next(scanner);
    cr_assert(SK_DIGIT == token.type);
    cr_assert(SK_DIGIT == skScanner_peek(scanner).type);

    token = skScanner_next(scanner);
    cr_assert(SK_COMMA == token.type);
    cr_assert(SK_COMMA == skScanner_peek(scanner).type);

    skScanner_skip_until(scanner, 1, SK_STRING);
    token = skScanner_peek(scanner);
    cr_assert(SK_STRING == token.type);
    cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
    cr_assert(SK_COLON == (token = skScanner_next(scanner)).type);
    cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
    cr_assert(SK_HYPHEN == (token = skScanner_next(scanner)).type);
    cr_assert(SK_DIGIT == (token = skScanner_next(scanner)).type);
    cr_assert(SK_DIGIT == (token = skScanner_next(scanner)).type);
    cr_assert(SK_DOT == (token = skScanner_next(scanner)).type);
    cr_assert(SK_DIGIT == (token = skScanner_next(scanner)).type);
    cr_assert(SK_DIGIT == (token = skScanner_next(scanner)).type);
    cr_assert(SK_DIGIT == (token = skScanner_next(scanner)).type);
    cr_assert(SK_EXP == (token = skScanner_next(scanner)).type);
    cr_assert(SK_PLUS == (token = skScanner_next(scanner)).type);
    cr_assert(SK_ZERO == (token = skScanner_next(scanner)).type);
    cr_assert(SK_ZERO == (token = skScanner_next(scanner)).type);
    cr_assert(SK_ZERO == (token = skScanner_next(scanner)).type);
    cr_assert(SK_DIGIT == (token = skScanner_next(scanner)).type);
    cr_assert(SK_DIGIT == (token = skScanner_next(scanner)).type);
    cr_assert(SK_COMMA == (token = skScanner_next(scanner)).type);
    cr_assert(SK_NL == (token = skScanner_next(scanner)).type);
    cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
    cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
    cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
    cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
    cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
    cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
    cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
    cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
    cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
    cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
    cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
    cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
    cr_assert(SK_STRING == (token = skScanner_next(scanner)).type);
    cr_assert(strncmp(token.lexeme.ptr, "numnum", token.lexeme.len) == 0);

    skScanner_skip_until(scanner, 1, SK_TRUE);
    token = skScanner_peek(scanner);
    cr_assert(SK_TRUE == token.type);
    cr_assert(strncmp(token.lexeme.ptr, "true", token.lexeme.len) == 0);
    cr_assert(SK_COMMA == (token = skScanner_next(scanner)).type);
    cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
    cr_assert(SK_STRING == (token = skScanner_next(scanner)).type);
    cr_assert(strncmp(token.lexeme.ptr, "asd", token.lexeme.len) == 0);
    cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
    cr_assert(SK_COLON == (token = skScanner_next(scanner)).type);
    cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
    cr_assert(SK_FALSE == (token = skScanner_next(scanner)).type);
    cr_assert(strncmp(token.lexeme.ptr, "false", token.lexeme.len) == 0);
    cr_assert(SK_COMMA == (token = skScanner_next(scanner)).type);
    cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
    cr_assert(SK_STRING == (token = skScanner_next(scanner)).type);
    cr_assert(strncmp(token.lexeme.ptr, "nulll", token.lexeme.len) == 0);
    cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
    cr_assert(SK_COLON == (token = skScanner_next(scanner)).type);
    cr_assert(SK_WS == (token = skScanner_next(scanner)).type);
    cr_assert(SK_NULL == (token = skScanner_next(scanner)).type);
    cr_assert(strncmp(token.lexeme.ptr, "null", token.lexeme.len) == 0);
    cr_assert(SK_COMMA == (token = skScanner_next(scanner)).type);
    skScanner_skip_until(scanner, 1, SK_ZERO);
    token = skScanner_peek(scanner);
    cr_assert(SK_ZERO == token.type);
    cr_assert(SK_DIGIT == (token = skScanner_next(scanner)).type);
    cr_assert(SK_DOT == (token = skScanner_next(scanner)).type);
    cr_assert(SK_DIGIT == (token = skScanner_next(scanner)).type);
    cr_assert(SK_DIGIT == (token = skScanner_next(scanner)).type);
    cr_assert(SK_COMMA == (token = skScanner_next(scanner)).type);
    cr_assert(SK_NL == (token = skScanner_next(scanner)).type);

    skScanner_skip_until(scanner, 1, SK_EOF);
    cr_assert(SK_EOF == skScanner_peek(scanner).type);
    cr_assert(SK_EOF == (token = skScanner_next(scanner)).type);
}

Test(SkJson, ParsePrimitives)
{
    cr_assert(scanner != NULL);

    skToken token = skScanner_next(scanner);
    cr_assert(SK_LCURLY == token.type);
    skScanner_next(scanner);

    skScanner_skip(scanner, 2, SK_WS, SK_NL);
    cr_assert(SK_STRING == (token = skScanner_peek(scanner)).type);

    skJsonNode* str_node = skparse_json_string(scanner, NULL);

    cr_assert(str_node != NULL);
    cr_assert_eq(strcmp(str_node->data.j_string, "glossary"), 0);
    cr_assert(str_node->type == SK_STRING_NODE);
    skJsonNode_drop(str_node);

    skScanner_skip_until(scanner, 1, SK_DIGIT);
    token = skScanner_peek(scanner);
    cr_assert(SK_DIGIT == token.type);

    skJsonNode* num_node = skparse_json_number(scanner, NULL);
    cr_assert(num_node != NULL);
    cr_assert_eq(num_node->data.j_int, 152);
    cr_assert_eq(num_node->type, SK_INT_NODE);
    skJsonNode_drop(num_node);

    skScanner_skip_until(scanner, 1, SK_HYPHEN);
    token = skScanner_peek(scanner);
    cr_assert_eq(token.type, SK_HYPHEN);

    skJsonNode* dbl_node = skparse_json_number(scanner, NULL);
    cr_assert(dbl_node->type == SK_DOUBLE_NODE);
    cr_assert(dbl_node->data.j_double == -12.523e15);
    skJsonNode_drop(dbl_node);

    skScanner_skip_until(scanner, 1, SK_TRUE);
    skJsonNode* bool_node = skparse_json_bool(scanner, NULL);
    cr_assert(bool_node != NULL);
    cr_assert(bool_node->type == SK_BOOL_NODE);
    cr_assert(bool_node->data.j_boolean == true);
    skJsonNode_drop(bool_node);

    skScanner_skip_until(scanner, 1, SK_FALSE);
    bool_node = skparse_json_bool(scanner, NULL);
    cr_assert(bool_node != NULL);
    cr_assert(bool_node->type == SK_BOOL_NODE);
    cr_assert(bool_node->data.j_boolean == false);
    skJsonNode_drop(bool_node);

    skScanner_skip_until(scanner, 1, SK_NULL);
    skJsonNode* null_node = skparse_json_null(scanner, NULL);
    cr_assert(null_node != NULL);
    cr_assert(null_node->type == SK_NULL_NODE);
    skJsonNode_drop(null_node);

    skScanner_skip_until(scanner, 1, SK_ZERO);
    cr_assert(SK_ZERO == (token = skScanner_peek(scanner)).type);

    skJsonNode* err_node = skparse_json_number(scanner, NULL);
    cr_assert(SK_ERROR_NODE == err_node->type);
    skJsonNode_drop(err_node);
}

void
json_setup_complex(void)
{
    printf("\n\n\n");
    if((fd = open("complex.json", 'r')) == -1) {
        fprintf(stderr, "Error opening complex.json\n");
        exit(EXIT_FAILURE);
    }

    n = read(fd, buf, BUFSIZ);

    scanner = skScanner_new(buf, n);
}

TestSuite(skJsonComplex, .init = json_setup_complex, .fini = json_teardown);

Test(skJsonComplex, ParseObjects)
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
    cr_assert(skVec_len(arr_node->data.j_array) == 7);
    skVec*      nodes = arr_node->data.j_array;
    skJsonNode* temp;

    cr_assert((temp = ((skJsonNode*) skVec_index(nodes, 0)))->type == SK_STRING_NODE);
    cr_assert_str_eq(temp->data.j_string, "one");
    cr_assert(temp->index == 0);
    cr_assert(temp->parent->index == 0);
    cr_assert(temp->parent->type == SK_ARRAY_NODE);
    cr_assert(temp->parent->parent == NULL);
    cr_assert(skVec_len(temp->parent->data.j_array) == 7);

    cr_assert((temp = ((skJsonNode*) skVec_index(nodes, 1)))->type == SK_STRING_NODE);
    cr_assert_str_eq(temp->data.j_string, "two");
    cr_assert(temp->index == 1);
    cr_assert(temp->parent->index == 0);
    cr_assert(temp->parent->type == SK_ARRAY_NODE);
    cr_assert(temp->parent->parent == NULL);
    cr_assert(skVec_len(temp->parent->data.j_array) == 7);

    cr_assert((temp = ((skJsonNode*) skVec_index(nodes, 2)))->type == SK_INT_NODE);
    cr_assert(temp->data.j_int == 3);
    cr_assert(temp->index == 2);
    cr_assert(temp->parent->index == 0);
    cr_assert(temp->parent->type == SK_ARRAY_NODE);
    cr_assert(temp->parent->parent == NULL);
    cr_assert(skVec_len(temp->parent->data.j_array) == 7);

    cr_assert((temp = ((skJsonNode*) skVec_index(nodes, 3)))->type == SK_DOUBLE_NODE);
    cr_assert(temp->data.j_double == 4.0e+1);
    cr_assert(temp->index == 3);
    cr_assert(temp->parent->index == 0);
    cr_assert(temp->parent->type == SK_ARRAY_NODE);
    cr_assert(temp->parent->parent == NULL);
    cr_assert(skVec_len(temp->parent->data.j_array) == 7);

    cr_assert((temp = ((skJsonNode*) skVec_index(nodes, 4)))->type == SK_BOOL_NODE);
    cr_assert(temp->data.j_boolean == true);
    cr_assert(temp->index == 4);
    cr_assert(temp->parent->index == 0);
    cr_assert(temp->parent->type == SK_ARRAY_NODE);
    cr_assert(temp->parent->parent == NULL);
    cr_assert(skVec_len(temp->parent->data.j_array) == 7);

    cr_assert((temp = ((skJsonNode*) skVec_index(nodes, 5)))->type == SK_BOOL_NODE);
    cr_assert(temp->data.j_boolean == false);
    cr_assert(temp->index == 5);
    cr_assert(temp->parent->index == 0);
    cr_assert(temp->parent->type == SK_ARRAY_NODE);
    cr_assert(temp->parent->parent == NULL);
    cr_assert(skVec_len(temp->parent->data.j_array) == 7);

    cr_assert((temp = ((skJsonNode*) skVec_index(nodes, 6)))->type == SK_NULL_NODE);
    cr_assert(temp->index == 6);
    cr_assert(temp->parent->index == 0);
    cr_assert(temp->parent->type == SK_ARRAY_NODE);
    cr_assert(temp->parent->parent == NULL);
    cr_assert(skVec_len(temp->parent->data.j_array) == 7);
    skJsonNode_drop(arr_node);

    cr_assert(skScanner_peek(scanner).type == SK_COMMA);
    cr_assert((token = skScanner_next(scanner)).type == SK_NL);
    cr_assert((token = skScanner_next(scanner)).type == SK_WS);
    cr_assert((token = skScanner_next(scanner)).type == SK_WS);
    cr_assert((token = skScanner_next(scanner)).type == SK_WS);
    cr_assert((token = skScanner_next(scanner)).type == SK_WS);
    cr_assert((token = skScanner_next(scanner)).type == SK_STRING);

    skJsonNode* json_string = skJsonNode_new(scanner, NULL);
    cr_assert(json_string != NULL);
    cr_assert(json_string->type == SK_STRING_NODE);
    cr_assert_str_eq(json_string->data.j_string, "obj");
    cr_assert(json_string->index == 0);
    cr_assert(json_string->parent == NULL);
    skJsonNode_drop(json_string);

    cr_assert((token = skScanner_next(scanner)).type == SK_COLON);
    cr_assert((token = skScanner_next(scanner)).type == SK_WS);
    cr_assert((token = skScanner_next(scanner)).type == SK_LCURLY);

    skJsonNode* json_object = skJsonNode_new(scanner, NULL);
    cr_assert_neq(json_object, NULL);
    cr_assert_eq(json_object->type, SK_OBJECT_NODE);
    cr_assert(json_object->index == 0);
    cr_assert(json_object->parent == NULL);

    skHashTable* table = json_object->data.j_object;
    cr_assert(skHashTable_len(table) == 5);

    skJsonNode* value = skHashTable_get(table, "three");
    cr_assert(value->data.j_int == 3);
    cr_assert(value->type == SK_INT_NODE);

    value = skHashTable_get(table, "one");
    cr_assert(value->data.j_int == 1);
    cr_assert(value->type == SK_INT_NODE);

    value = skHashTable_get(table, "simple");
    cr_assert(value->data.j_boolean == false);
    cr_assert(value->type == SK_BOOL_NODE);

    value = skHashTable_get(table, "two");
    cr_assert(value->data.j_int == 2);
    cr_assert(value->type == SK_INT_NODE);

    value = skHashTable_get(table, "complex");
    cr_assert(value->data.j_boolean == true);
    cr_assert(value->type == SK_BOOL_NODE);

    skJsonNode_drop(json_object);
}

Test(skJsonComplex, ParseWhole)
{
    cr_assert(scanner != NULL);

    skScanner_next(scanner);
    cr_assert(skScanner_peek(scanner).type == SK_LCURLY);
    skJsonNode* root = skJsonNode_new(scanner, NULL);
    cr_assert(root != NULL);
    cr_assert(root->type == SK_OBJECT_NODE);
    cr_assert(root->index == 0);
    cr_assert(root->parent == NULL);
    cr_assert(root->data.j_object != NULL);

    skHashTable* table = root->data.j_object;
    cr_assert_eq(skHashTable_len(table), 3);
    cr_assert(skHashTable_contains(table, "arr"));
    cr_assert(skHashTable_contains(table, "end"));
    cr_assert(skHashTable_contains(table, "obj"));

    skJsonNode_drop(root);
}

skJson* json_final;

void
setup_final(void)
{
    if((fd = open("meta_github.json", 'r')) == -1) {
        fprintf(stderr, "Error opening file 'meta_github.json'\n");
        exit(EXIT_FAILURE);
    }

    int buff_size = lseek(fd, 0, SEEK_END);

    if(buff_size == -1) {
        fprintf(stderr, "Error measuring file size of 'meta_github.json'\n");
        exit(EXIT_FAILURE);
    }

    if(lseek(fd, 0, SEEK_SET) == -1) {
        fprintf(stderr, "Error setting the offset to the start of the file\n");
        exit(EXIT_FAILURE);
    }

    if((n = read(fd, buf, buff_size)) == -1) {
        fprintf(stderr, "Error reading file 'meta_github.json'\n");
        exit(EXIT_FAILURE);
    }
}

void
teardown_final(void)
{
    skJson_drop(json_final);
    close(fd);
}

TestSuite(skJsonFinal, .init = setup_final, .fini = json_teardown);

Test(skJsonFinal, ParseComplete)
{
    json_final = sk_json_new(buf, n);
    cr_assert(json_final != NULL);
}
