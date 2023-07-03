#include "../src/skjson.h"
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
int          cmp_objtuples(skJsonNode* a, skJsonNode* b);

int cmp_objtuples(skJsonNode* a, skJsonNode* b)
{
    return strcmp(a->data.j_member.key, b->data.j_member.key);
}

skScanner* scanner;
char       buf[1000000];
int        fd;
int        n;

void json_setup(void)
{
    printf("\n\n\n");
    if((fd = open("test.json", 'r')) == -1) {
        fprintf(stderr, "Error opening test.json\n");
        exit(EXIT_FAILURE);
    }

    n = read(fd, buf, BUFSIZ);

    scanner = skScanner_new(buf, n);
}

void json_teardown(void)
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

void json_setup_complex(void)
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
    cr_assert(temp->parent->type == SK_ARRAY_NODE);
    cr_assert(temp->parent->parent == NULL);
    cr_assert(skVec_len(temp->parent->data.j_array) == 7);

    cr_assert((temp = ((skJsonNode*) skVec_index(nodes, 1)))->type == SK_STRING_NODE);
    cr_assert_str_eq(temp->data.j_string, "two");
    cr_assert(temp->parent->type == SK_ARRAY_NODE);
    cr_assert(temp->parent->parent == NULL);
    cr_assert(skVec_len(temp->parent->data.j_array) == 7);

    cr_assert((temp = ((skJsonNode*) skVec_index(nodes, 2)))->type == SK_INT_NODE);
    cr_assert(temp->data.j_int == 3);
    cr_assert(temp->parent->type == SK_ARRAY_NODE);
    cr_assert(temp->parent->parent == NULL);
    cr_assert(skVec_len(temp->parent->data.j_array) == 7);

    cr_assert((temp = ((skJsonNode*) skVec_index(nodes, 3)))->type == SK_DOUBLE_NODE);
    cr_assert(temp->data.j_double == 4.0e+1);
    cr_assert(temp->parent->type == SK_ARRAY_NODE);
    cr_assert(temp->parent->parent == NULL);
    cr_assert(skVec_len(temp->parent->data.j_array) == 7);

    cr_assert((temp = ((skJsonNode*) skVec_index(nodes, 4)))->type == SK_BOOL_NODE);
    cr_assert(temp->data.j_boolean == true);
    cr_assert(temp->parent->type == SK_ARRAY_NODE);
    cr_assert(temp->parent->parent == NULL);
    cr_assert(skVec_len(temp->parent->data.j_array) == 7);

    cr_assert((temp = ((skJsonNode*) skVec_index(nodes, 5)))->type == SK_BOOL_NODE);
    cr_assert(temp->data.j_boolean == false);
    cr_assert(temp->parent->type == SK_ARRAY_NODE);
    cr_assert(temp->parent->parent == NULL);
    cr_assert(skVec_len(temp->parent->data.j_array) == 7);

    cr_assert((temp = ((skJsonNode*) skVec_index(nodes, 6)))->type == SK_NULL_NODE);
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

    skJsonNode* json_string = skJsonNode_parse(scanner, NULL);
    cr_assert(json_string != NULL);
    cr_assert(json_string->type == SK_STRING_NODE);
    cr_assert_str_eq(json_string->data.j_string, "obj");
    cr_assert(json_string->parent == NULL);
    skJsonNode_drop(json_string);

    cr_assert((token = skScanner_next(scanner)).type == SK_COLON);
    cr_assert((token = skScanner_next(scanner)).type == SK_WS);
    cr_assert((token = skScanner_next(scanner)).type == SK_LCURLY);

    skJsonNode* json_object = skJsonNode_parse(scanner, NULL);
    cr_assert_neq(json_object, NULL);
    cr_assert_eq(json_object->type, SK_OBJECT_NODE);
    cr_assert(json_object->parent == NULL);

    skVec* table = json_object->data.j_object;
    cr_assert(skVec_len(table) == 5);

    skJsonNode dummynode;
    dummynode.data.j_member.key = "three";
    skJsonNode* value = skVec_get_by_key(table, &dummynode, (CmpFn) cmp_objtuples, false);
    cr_assert(value->data.j_member.value->data.j_int == 3);
    cr_assert(value->data.j_member.value->type == SK_INT_NODE);

    dummynode.data.j_member.key = "one";
    value = skVec_get_by_key(table, &dummynode, (CmpFn) cmp_objtuples, false);
    cr_assert(value->data.j_member.value->data.j_int == 1);
    cr_assert(value->data.j_member.value->type == SK_INT_NODE);

    dummynode.data.j_member.key = "simple";
    value = skVec_get_by_key(table, &dummynode, (CmpFn) cmp_objtuples, false);
    cr_assert(value->data.j_member.value->data.j_boolean == false);
    cr_assert(value->data.j_member.value->type == SK_BOOL_NODE);

    dummynode.data.j_member.key = "two";
    value = skVec_get_by_key(table, &dummynode, (CmpFn) cmp_objtuples, false);
    cr_assert(value->data.j_member.value->data.j_int == 2);
    cr_assert(value->data.j_member.value->type == SK_INT_NODE);

    dummynode.data.j_member.key = "complex";
    value = skVec_get_by_key(table, &dummynode, (CmpFn) cmp_objtuples, false);
    cr_assert(value->data.j_member.value->data.j_boolean == true);
    cr_assert(value->data.j_member.value->type == SK_BOOL_NODE);

    skJsonNode_drop(json_object);
}

Test(skJsonComplex, ParseWhole)
{
    cr_assert(scanner != NULL);

    skScanner_next(scanner);
    cr_assert(skScanner_peek(scanner).type == SK_LCURLY);

    skJsonNode* root = skJsonNode_parse(scanner, NULL);
    cr_assert(root != NULL);
    cr_assert(root->type == SK_OBJECT_NODE);
    cr_assert(root->parent == NULL);
    cr_assert(root->data.j_object != NULL);

    skVec* table = root->data.j_object;
    cr_assert_eq(skVec_len(table), 3);
    skJsonNode dummynode;
    dummynode.data.j_member.key = "arr";
    cr_assert(skVec_contains(table, &dummynode, (CmpFn) cmp_objtuples, false));
    dummynode.data.j_member.key = "obj";
    cr_assert(skVec_contains(table, &dummynode, (CmpFn) cmp_objtuples, false));
    dummynode.data.j_member.key = "end";
    cr_assert(skVec_contains(table, &dummynode, (CmpFn) cmp_objtuples, false));

    skJsonNode_drop(root);
}

skJson* json_final;

void setup_final(void)
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

void teardown_final(void)
{
    skJson_drop(&json_final);
    close(fd);
}

TestSuite(skJsonFinal, .init = setup_final, .fini = json_teardown);

Test(skJsonFinal, ParseComplete)
{
    skVec*      inner;
    skJsonNode* objnode;
    skJsonNode* current;
    skJsonNode* temp;
    char*       tempkey;
    // unsigned char* out;

    json_final = skJson_parse(buf, n);
    cr_assert(json_final != NULL);
    cr_assert(skJson_type(json_final) == SK_OBJECT_NODE);
    cr_assert(skJson_parent(json_final) == NULL);

    inner = json_final->data.j_object;
    cr_assert_eq(skVec_len(inner), 13);

    cr_assert_eq((current = skVec_front(inner))->type, SK_MEMBER_NODE);
    cr_assert(skJson_type(current) == SK_MEMBER_NODE);
    cr_assert_eq(current->data.j_member.value->type, SK_BOOL_NODE);
    cr_assert(skJson_type(current->data.j_member.value) == SK_BOOL_NODE);
    cr_assert_eq(current->data.j_member.value->data.j_boolean, false);
    cr_assert_eq(skJson_bool_value(current->data.j_member.value), false);
    skJson_bool_set(current->data.j_member.value, true);
    cr_assert_eq(skJson_bool_value(current->data.j_member.value), true);
    cr_assert_str_eq(current->data.j_member.key, "verifiable_password_authentication");
    cr_assert_eq(skJson_bool_value(skJson_member_value(current)), true);
    cr_assert_str_eq(
        (tempkey = skJson_member_key(current)),
        "verifiable_password_authentication");
    free(tempkey);

    cr_assert_eq((current = skVec_index(inner, 1))->type, SK_MEMBER_NODE);
    cr_assert(skJson_type(current) == SK_MEMBER_NODE);
    cr_assert_eq(current->data.j_member.value->type, SK_OBJECT_NODE);
    cr_assert(skJson_type(current->data.j_member.value) == SK_OBJECT_NODE);
    cr_assert_str_eq(current->data.j_member.key, "ssh_key_fingerprints");
    cr_assert_str_eq((tempkey = skJson_member_key(current)), "ssh_key_fingerprints");
    free(tempkey);
    cr_assert_neq((objnode = skJson_member_value(current)), NULL);
    cr_assert_eq(skJson_object_len(objnode), 3);

    cr_assert_eq((temp = skJson_object_element(objnode, 0))->type, SK_MEMBER_NODE);
    cr_assert(skJson_type(temp) == SK_MEMBER_NODE);
    cr_assert_str_eq((tempkey = skJson_member_key(temp)), "SHA256_ECDSA");
    free(tempkey);
    cr_assert_eq(skJson_type(skJson_member_value(temp)), SK_STRING_NODE);
    cr_assert_str_eq(
        temp->data.j_member.value->data.j_string,
        "p2QAMXNIC1TJYWeIOttrVc98/R1BUFWu3/LiyKgUfQM");
    cr_assert_str_eq(
        (tempkey = skJson_string_value(temp->data.j_member.value)),
        "p2QAMXNIC1TJYWeIOttrVc98/R1BUFWu3/LiyKgUfQM");
    free(tempkey);
    cr_assert_str_eq(
        (tempkey = skJson_string_value(skJson_member_value(temp))),
        "p2QAMXNIC1TJYWeIOttrVc98/R1BUFWu3/LiyKgUfQM");
    free(tempkey);

    cr_assert_eq((temp = skJson_object_element(objnode, 1))->type, SK_MEMBER_NODE);
    cr_assert_str_eq(temp->data.j_member.key, "SHA256_ED25519");
    cr_assert_str_eq((tempkey = skJson_member_key(temp)), "SHA256_ED25519");
    free(tempkey);
    cr_assert_eq(temp->data.j_member.value->type, SK_STRING_NODE);
    cr_assert(skJson_type(temp->data.j_member.value) == SK_STRING_NODE);
    cr_assert(skJson_type(skJson_member_value(temp)) == SK_STRING_NODE);
    cr_assert_str_eq(
        temp->data.j_member.value->data.j_string,
        "+DiY3wvvV6TuJJhbpZisF/zLDA0zPMSvHdkr4UvCOqU");
    cr_assert_str_eq(
        (tempkey = skJson_string_value(temp->data.j_member.value)),
        "+DiY3wvvV6TuJJhbpZisF/zLDA0zPMSvHdkr4UvCOqU");
    free(tempkey);
    cr_assert_str_eq(
        (tempkey = skJson_string_value(skJson_member_value(temp))),
        "+DiY3wvvV6TuJJhbpZisF/zLDA0zPMSvHdkr4UvCOqU");
    free(tempkey);

    cr_assert_eq((temp = skJson_object_element(objnode, 2))->type, SK_MEMBER_NODE);
    cr_assert(skJson_type(temp) == SK_MEMBER_NODE);
    cr_assert_str_eq(temp->data.j_member.key, "SHA256_RSA");
    cr_assert_str_eq((tempkey = skJson_member_key(temp)), "SHA256_RSA");
    free(tempkey);
    cr_assert_eq(temp->data.j_member.value->type, SK_STRING_NODE);
    cr_assert(skJson_type(temp->data.j_member.value) == SK_STRING_NODE);
    cr_assert(skJson_type(skJson_member_value(temp)) == SK_STRING_NODE);
    cr_assert_str_eq(
        temp->data.j_member.value->data.j_string,
        "uNiVztksCsDhcc0u9e8BujQXVUpKZIDTMczCvj3tD2s");
    cr_assert_str_eq(
        (tempkey = skJson_string_value(temp->data.j_member.value)),
        "uNiVztksCsDhcc0u9e8BujQXVUpKZIDTMczCvj3tD2s");
    free(tempkey);
    cr_assert_str_eq(
        (tempkey = skJson_string_value(skJson_member_value(temp))),
        "uNiVztksCsDhcc0u9e8BujQXVUpKZIDTMczCvj3tD2s");
    free(tempkey);
    cr_assert_eq(skJson_object_element(objnode, 3), NULL);

    cr_assert((objnode = skJson_object_element(json_final, 2)) != NULL);
    cr_assert_eq(skJson_type(objnode), SK_MEMBER_NODE);
    cr_assert_str_eq((tempkey = skJson_member_key(objnode)), "ssh_keys");
    free(tempkey);
    cr_assert_eq(skJson_type(temp = skJson_member_value(objnode)), SK_ARRAY_NODE);
    cr_assert_eq(skJson_array_len(temp), 3);
    cr_assert_eq(skJson_array_front(temp)->type, SK_STRING_NODE);
    cr_assert_eq(skJson_array_back(temp)->type, SK_STRING_NODE);
    cr_assert_eq(skJson_array_index(temp, 1)->type, SK_STRING_NODE);
    cr_assert_eq(skJson_array_push_null(temp), true);
    cr_assert_eq(skJson_array_back(temp)->type, SK_NULL_NODE);
    cr_assert_eq(skJson_array_insert_bool(temp, false, 0), true);
    cr_assert_eq(skJson_array_front(temp)->type, SK_BOOL_NODE);
    cr_assert_eq(skJson_bool_value(skJson_array_front(temp)), false);
    cr_assert_eq(skJson_array_back(temp)->type, SK_NULL_NODE);
    cr_assert_eq(skJson_array_len(temp), 5);
    skJson_array_clear(temp);
    cr_assert_eq(skJson_array_len(temp), 0);
    cr_assert(skJson_array_push_strlit(
        temp,
        "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIOMqqnkVzrm0SdG6UOoqKLsabgH5C9okWi0dh2l9GKJl"));
    cr_assert(skJson_array_push_strlit(
        temp,
        "ecdsa-sha2-nistp256 "
        "AAAAE2VjZHNhLXNoYTItbmlzdHAyNTYAAAAIbmlzdHAyNTYAAABBBEmKSENjQEezOmxkZMy7opKgwFB9nkt5Y"
        "RrYMjNuG5N87uRgg6CLrbo5wAdT/y6v0mKV0U2w0WZ2YB/++Tpockg="));
    cr_assert(skJson_array_push_strlit(
        temp,
        "ssh-rsa "
        "AAAAB3NzaC1yc2EAAAADAQABAAABgQCj7ndNxQowgcQnjshcLrqPEiiphnt+"
        "VTTvDP6mHBL9j1aNUkY4Ue1gvwnGLVlOhGeYrnZaMgRK6+PKCUXaDbC7qtbW8gIkhL7aGCsOr/C56SJMy/"
        "BCZfxd1nWzAOxSDPgVsmerOBYfNqltV9/"
        "hWCqBywINIR+5dIg6JTJ72pcEpEjcYgXkE2YEFXV1JHnsKgbLWNlhScqb2UmyRkQyytRLtL+38TGxkxCflmO+"
        "5Z8CSSNY7GidjMIZ7Q4zMjA2n1nGrlTDkzwDCsw+"
        "wqFPGQA179cnfGWOWRVruj16z6XyvxvjJwbz0wQZ75XK5tKSb7FNyeIEs4TT4jk+S4dhPeAUC5y+"
        "bDYirYgM4GC7uEnztnZyaVWQ7B381AK4Qdrwt51ZqExKbQpTUNn+EjqoTwvqNj4kqx5QUCI0ThS/"
        "YkOxJCXmPUWZbhjpCg56i+2aB6CmK2JGhn57K5mj0MNdBXA4/"
        "WnwH6XoPWJzK5Nyu2zB3nAZp+S5hpQs+p1vN1/wsjk="));
    cr_assert_eq(skJson_array_len(temp), 3);
    cr_assert_eq(skJson_array_front(temp)->type, SK_STRINGLIT_NODE);
    cr_assert_eq(skJson_array_back(temp)->type, SK_STRINGLIT_NODE);
    cr_assert_eq(skJson_array_index(temp, 1)->type, SK_STRINGLIT_NODE);

    // out = skJson_serialize(json_final);
    // cr_assert(out != NULL);
    // printf("\n\n");
    // printf("%s\n\n", out);
}
