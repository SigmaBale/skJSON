/* clang-format off */
#include "../src/skparser.h"
#include "../src/skjson.h"
#include <criterion/criterion.h>
#include <fcntl.h>
#include <unistd.h>
/* clang-format on */

skJsonString skJsonString_new_internal(skScanner* scanner, skJson* err_node);
skJson       skparse_json_object(skScanner* scanner, skJson* parent);
skJson       skparse_json_array(skScanner* scanner, skJson* parent);
skJson       skparse_json_string(skScanner* scanner, skJson* parent);
skJson       skparse_json_number(skScanner* scanner, skJson* parent);
skJson       skparse_json_bool(skScanner* scanner, skJson* parent);
skJson       skparse_json_null(skScanner* scanner, skJson* parent);
int          cmp_tuples(skObjTuple* a, skObjTuple* b);
int          cmp_tuples_descending(skObjTuple* a, skObjTuple* b);

int cmp_tuples(skObjTuple* a, skObjTuple* b)
{
    return strcmp(a->key, b->key);
}

int cmp_tuples_descending(skObjTuple* a, skObjTuple* b)
{
    int cmp;
    cmp = strcmp(a->key, b->key);

    if(cmp > 0) {
        return -1;
    } else if(cmp < 0) {
        return 1;
    }

    return 0;
}

skScanner* scanner;
char       buf[1000000];
int        fd;
int        n;

void json_setup(void)
{
    if((fd = open("error.json", 'r')) == -1) {
        fprintf(stderr, "Error opening error.json\n");
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

    skJson str_node = skparse_json_string(scanner, NULL);
    cr_assert(str_node.type != SK_NONE_NODE);
    cr_assert_eq(strcmp(str_node.data.j_string, "glossary"), 0);
    cr_assert(str_node.type == SK_STRING_NODE);
    skJson_drop(&str_node);
    cr_assert(str_node.type == SK_NONE_NODE);

    skScanner_skip_until(scanner, 1, SK_DIGIT);
    token = skScanner_peek(scanner);
    cr_assert(SK_DIGIT == token.type);

    skJson num_node = skparse_json_number(scanner, NULL);
    cr_assert_eq(num_node.data.j_int, 152);
    cr_assert_eq(num_node.type, SK_INT_NODE);
    skJson_drop(&num_node);
    cr_assert(num_node.type == SK_NONE_NODE);

    skScanner_skip_until(scanner, 1, SK_HYPHEN);
    token = skScanner_peek(scanner);
    cr_assert_eq(token.type, SK_HYPHEN);

    skJson dbl_node = skparse_json_number(scanner, NULL);
    cr_assert(dbl_node.type == SK_DOUBLE_NODE);
    cr_assert(dbl_node.data.j_double == -12.523e15);
    skJson_drop(&dbl_node);
    cr_assert(dbl_node.type == SK_NONE_NODE);

    skScanner_skip_until(scanner, 1, SK_TRUE);
    skJson bool_node = skparse_json_bool(scanner, NULL);
    cr_assert(bool_node.type == SK_BOOL_NODE);
    cr_assert(bool_node.data.j_boolean == true);
    skJson_drop(&bool_node);
    cr_assert(bool_node.type == SK_NONE_NODE);

    skScanner_skip_until(scanner, 1, SK_FALSE);
    bool_node = skparse_json_bool(scanner, NULL);
    cr_assert(bool_node.type == SK_BOOL_NODE);
    cr_assert(bool_node.data.j_boolean == false);
    skJson_drop(&bool_node);
    cr_assert(bool_node.type == SK_NONE_NODE);

    skScanner_skip_until(scanner, 1, SK_NULL);
    skJson null_node = skparse_json_null(scanner, NULL);
    cr_assert(null_node.type == SK_NULL_NODE);
    skJson_drop(&null_node);
    cr_assert(null_node.type == SK_NONE_NODE);

    skScanner_skip_until(scanner, 1, SK_ZERO);
    cr_assert(SK_ZERO == (token = skScanner_peek(scanner)).type);

    skJson err_node = skparse_json_number(scanner, NULL);
    cr_assert(SK_ERROR_NODE == err_node.type);
    skJson_drop(&err_node);
    cr_assert(err_node.type == SK_NONE_NODE);
}

void json_setup_simple(void)
{
    if((fd = open("simple.json", 'r')) == -1) {
        fprintf(stderr, "Error opening simple.json\n");
        exit(EXIT_FAILURE);
    }

    n = read(fd, buf, BUFSIZ);

    scanner = skScanner_new(buf, n);
}

TestSuite(skJsonComplex, .init = json_setup_simple, .fini = json_teardown);

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

    skJson arr_node = skparse_json_array(scanner, NULL);
    cr_assert(arr_node.type != SK_NONE_NODE);
    cr_assert(arr_node.type == SK_ARRAY_NODE);
    cr_assert(skVec_len(arr_node.data.j_array) == 7);
    skVec*  nodes = arr_node.data.j_array;
    skJson* temp;

    cr_assert((temp = ((skJson*) skVec_index(nodes, 0)))->type == SK_STRING_NODE);
    cr_assert_str_eq(temp->data.j_string, "one");
    cr_assert(temp->parent_arena.type == SK_ARRAY_NODE);
    cr_assert(temp->parent_arena.ptr != NULL);
    cr_assert(skVec_len((skVec*) temp->parent_arena.ptr) == 7);

    cr_assert((temp = ((skJson*) skVec_index(nodes, 1)))->type == SK_STRING_NODE);
    cr_assert_str_eq(temp->data.j_string, "two");
    cr_assert(temp->parent_arena.type == SK_ARRAY_NODE);
    cr_assert(temp->parent_arena.ptr != NULL);
    cr_assert(skVec_len((skVec*) temp->parent_arena.ptr) == 7);

    cr_assert((temp = ((skJson*) skVec_index(nodes, 2)))->type == SK_INT_NODE);
    cr_assert(temp->data.j_int == 3);
    cr_assert(temp->parent_arena.type == SK_ARRAY_NODE);
    cr_assert(temp->parent_arena.ptr != NULL);
    cr_assert(skVec_len((skVec*) temp->parent_arena.ptr) == 7);

    cr_assert((temp = ((skJson*) skVec_index(nodes, 3)))->type == SK_DOUBLE_NODE);
    cr_assert(temp->data.j_double == 4.0e+1);
    cr_assert(temp->parent_arena.type == SK_ARRAY_NODE);
    cr_assert(temp->parent_arena.ptr != NULL);
    cr_assert(skVec_len((skVec*) temp->parent_arena.ptr) == 7);

    cr_assert((temp = ((skJson*) skVec_index(nodes, 4)))->type == SK_BOOL_NODE);
    cr_assert(temp->data.j_boolean == true);
    cr_assert(temp->parent_arena.type == SK_ARRAY_NODE);
    cr_assert(temp->parent_arena.ptr != NULL);
    cr_assert(skVec_len((skVec*) temp->parent_arena.ptr) == 7);

    cr_assert((temp = ((skJson*) skVec_index(nodes, 5)))->type == SK_BOOL_NODE);
    cr_assert(temp->data.j_boolean == false);
    cr_assert(temp->parent_arena.type == SK_ARRAY_NODE);
    cr_assert(temp->parent_arena.ptr != NULL);
    cr_assert(skVec_len((skVec*) temp->parent_arena.ptr) == 7);

    cr_assert((temp = ((skJson*) skVec_index(nodes, 6)))->type == SK_NULL_NODE);
    cr_assert(temp->parent_arena.type == SK_ARRAY_NODE);
    cr_assert(temp->parent_arena.ptr != NULL);
    cr_assert(skVec_len((skVec*) temp->parent_arena.ptr) == 7);
    skJson_drop(&arr_node);
    cr_assert(arr_node.type == SK_NONE_NODE);

    cr_assert(skScanner_peek(scanner).type == SK_COMMA);
    cr_assert((token = skScanner_next(scanner)).type == SK_NL);
    cr_assert((token = skScanner_next(scanner)).type == SK_WS);
    cr_assert((token = skScanner_next(scanner)).type == SK_WS);
    cr_assert((token = skScanner_next(scanner)).type == SK_WS);
    cr_assert((token = skScanner_next(scanner)).type == SK_WS);
    cr_assert((token = skScanner_next(scanner)).type == SK_STRING);

    skJson json_string = skJsonNode_parse(scanner, NULL);
    cr_assert(json_string.type != SK_NONE_NODE);
    cr_assert(json_string.type == SK_STRING_NODE);
    cr_assert_str_eq(json_string.data.j_string, "obj");
    cr_assert(json_string.parent_arena.ptr == NULL);
    skJson_drop(&json_string);
    cr_assert(json_string.type == SK_NONE_NODE);

    cr_assert((token = skScanner_next(scanner)).type == SK_COLON);
    cr_assert((token = skScanner_next(scanner)).type == SK_WS);
    cr_assert((token = skScanner_next(scanner)).type == SK_LCURLY);

    skJson json_object = skJsonNode_parse(scanner, NULL);
    cr_assert(json_object.type != SK_NONE_NODE);
    cr_assert_eq(json_object.type, SK_OBJECT_NODE);
    cr_assert(json_object.parent_arena.ptr == NULL);

    skVec* table = json_object.data.j_object;
    cr_assert(skVec_len(table) == 5);

    skObjTuple dummytuple;
    dummytuple.key    = "three";
    skObjTuple* tuple = skVec_get_by_key(table, &dummytuple, (CmpFn) cmp_tuples, false);
    cr_assert(tuple->value.data.j_int == 3);
    cr_assert(tuple->value.type == SK_INT_NODE);

    dummytuple.key = "one";
    tuple          = skVec_get_by_key(table, &dummytuple, (CmpFn) cmp_tuples, false);
    cr_assert(tuple->value.data.j_int == 1);
    cr_assert(tuple->value.type == SK_INT_NODE);

    dummytuple.key = "simple";
    tuple          = skVec_get_by_key(table, &dummytuple, (CmpFn) cmp_tuples, false);
    cr_assert(tuple->value.data.j_boolean == false);
    cr_assert(tuple->value.type == SK_BOOL_NODE);

    dummytuple.key = "two";
    tuple          = skVec_get_by_key(table, &dummytuple, (CmpFn) cmp_tuples, false);
    cr_assert(tuple->value.data.j_int == 2);
    cr_assert(tuple->value.type == SK_INT_NODE);

    dummytuple.key = "complex";
    tuple          = skVec_get_by_key(table, &dummytuple, (CmpFn) cmp_tuples, false);
    cr_assert(tuple->value.data.j_boolean == true);
    cr_assert(tuple->value.type == SK_BOOL_NODE);

    skJson_drop(&json_object);
    cr_assert(json_object.type = SK_NONE_NODE);
}

Test(skJsonComplex, ParseWhole)
{
    cr_assert(scanner != NULL);

    skScanner_next(scanner);
    cr_assert(skScanner_peek(scanner).type == SK_LCURLY);

    skJson root = skJsonNode_parse(scanner, NULL);
    cr_assert(root.type != SK_NONE_NODE);
    cr_assert(root.type == SK_OBJECT_NODE);
    cr_assert(root.parent_arena.ptr == NULL);
    cr_assert(root.data.j_object != NULL);

    skVec* table = root.data.j_object;
    cr_assert_eq(skVec_len(table), 3);
    skObjTuple dummytuple;
    dummytuple.key = "arr";
    cr_assert(skVec_contains(table, &dummytuple, (CmpFn) cmp_tuples, false));
    dummytuple.key = "obj";
    cr_assert(skVec_contains(table, &dummytuple, (CmpFn) cmp_tuples, false));
    dummytuple.key = "end";
    cr_assert(skVec_contains(table, &dummytuple, (CmpFn) cmp_tuples, false));

    skJson_drop(&root);
    cr_assert(root.type == SK_NONE_NODE);
}

skJson json_final;

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
    skJson*     objnode;
    skObjTuple* current;
    skObjTuple* temp;
    char*       tempkey;
    // unsigned char* out;

    json_final = skJson_parse(buf, n);
    cr_assert(json_final.type != SK_ERROR_NODE);
    cr_assert(skJson_type(&json_final) == SK_OBJECT_NODE);
    cr_assert(skJson_parent(&json_final) == false);

    inner = json_final.data.j_object;
    cr_assert_eq(skVec_len(inner), 13);

    cr_assert((current = skVec_index(inner, 0)) != NULL);
    cr_assert_eq(current->value.type, SK_BOOL_NODE);
    cr_assert(skJson_type(&current->value) == SK_BOOL_NODE);
    cr_assert_eq(current->value.data.j_boolean, false);
    cr_assert_eq(skJson_bool_value(&current->value), false);
    skJson_bool_set(&current->value, true);
    cr_assert_eq(skJson_bool_value(&current->value), true);
    cr_assert_str_eq(current->key, "verifiable_password_authentication");
    cr_assert_eq(skJson_bool_value(skJson_objtuple_value(current)), true);
    cr_assert_str_eq(
        (tempkey = skJson_objtuple_key(current)),
        "verifiable_password_authentication");
    free(tempkey);

    cr_assert((current = skVec_index(inner, 1)) != NULL);
    cr_assert_eq(current->value.type, SK_OBJECT_NODE);
    cr_assert(skJson_type(&current->value) == SK_OBJECT_NODE);
    cr_assert_str_eq(current->key, "ssh_key_fingerprints");
    cr_assert_str_eq((tempkey = skJson_objtuple_key(current)), "ssh_key_fingerprints");
    free(tempkey);
    cr_assert_neq((objnode = skJson_objtuple_value(current)), NULL);
    cr_assert_eq(skJson_object_len(objnode), 3);

    cr_assert((temp = skJson_object_index(objnode, 0)) != NULL);
    cr_assert_str_eq((tempkey = skJson_objtuple_key(temp)), "SHA256_ECDSA");
    free(tempkey);
    cr_assert_eq(skJson_type(skJson_objtuple_value(temp)), SK_STRING_NODE);
    cr_assert_str_eq(temp->value.data.j_string, "p2QAMXNIC1TJYWeIOttrVc98/R1BUFWu3/LiyKgUfQM");
    cr_assert_str_eq(
        (tempkey = skJson_string_value(&temp->value)),
        "p2QAMXNIC1TJYWeIOttrVc98/R1BUFWu3/LiyKgUfQM");
    free(tempkey);
    cr_assert_str_eq(
        (tempkey = skJson_string_value(skJson_objtuple_value(temp))),
        "p2QAMXNIC1TJYWeIOttrVc98/R1BUFWu3/LiyKgUfQM");
    free(tempkey);

    cr_assert((temp = skJson_object_index(objnode, 1)) != NULL);
    cr_assert_str_eq(temp->key, "SHA256_ED25519");
    cr_assert_str_eq((tempkey = skJson_objtuple_key(temp)), "SHA256_ED25519");
    free(tempkey);
    cr_assert_eq(temp->value.type, SK_STRING_NODE);
    cr_assert(skJson_type(&temp->value) == SK_STRING_NODE);
    cr_assert(skJson_type(skJson_objtuple_value(temp)) == SK_STRING_NODE);
    cr_assert_str_eq(temp->value.data.j_string, "+DiY3wvvV6TuJJhbpZisF/zLDA0zPMSvHdkr4UvCOqU");
    cr_assert_str_eq(
        (tempkey = skJson_string_value(&temp->value)),
        "+DiY3wvvV6TuJJhbpZisF/zLDA0zPMSvHdkr4UvCOqU");
    free(tempkey);
    cr_assert_str_eq(
        (tempkey = skJson_string_value(skJson_objtuple_value(temp))),
        "+DiY3wvvV6TuJJhbpZisF/zLDA0zPMSvHdkr4UvCOqU");
    free(tempkey);

    cr_assert((temp = skJson_object_index(objnode, 2)) != NULL);
    cr_assert_str_eq(temp->key, "SHA256_RSA");
    cr_assert_str_eq((tempkey = skJson_objtuple_key(temp)), "SHA256_RSA");
    free(tempkey);
    cr_assert_eq(temp->value.type, SK_STRING_NODE);
    cr_assert(skJson_type(&temp->value) == SK_STRING_NODE);
    cr_assert(skJson_type(skJson_objtuple_value(temp)) == SK_STRING_NODE);
    cr_assert_str_eq(temp->value.data.j_string, "uNiVztksCsDhcc0u9e8BujQXVUpKZIDTMczCvj3tD2s");
    cr_assert_str_eq(
        (tempkey = skJson_string_value(&temp->value)),
        "uNiVztksCsDhcc0u9e8BujQXVUpKZIDTMczCvj3tD2s");
    free(tempkey);
    cr_assert_str_eq(
        (tempkey = skJson_string_value(skJson_objtuple_value(temp))),
        "uNiVztksCsDhcc0u9e8BujQXVUpKZIDTMczCvj3tD2s");
    free(tempkey);
    cr_assert_eq(skJson_object_index(objnode, 3), NULL);

    cr_assert((temp = skJson_object_index(&json_final, 2)) != NULL);
    cr_assert_str_eq((tempkey = skJson_objtuple_key(temp)), "ssh_keys");
    free(tempkey);
    skJson* ret;
    cr_assert_eq(skJson_type(ret = skJson_objtuple_value(temp)), SK_ARRAY_NODE);
    cr_assert_eq(skJson_array_len(ret), 3);
    cr_assert_eq(skJson_array_front(ret)->type, SK_STRING_NODE);
    cr_assert_eq(skJson_array_back(ret)->type, SK_STRING_NODE);
    cr_assert_eq(skJson_array_index(ret, 1)->type, SK_STRING_NODE);
    cr_assert_eq(skJson_array_push_null(ret), true);
    cr_assert_eq(skJson_array_len(ret), 4);
    cr_assert_eq(skJson_array_back(ret)->type, SK_NULL_NODE);
    cr_assert_eq(skJson_array_insert_bool(ret, false, 0), true);
    cr_assert_eq(skJson_array_len(ret), 5);
    cr_assert_eq(skJson_array_front(ret)->type, SK_BOOL_NODE);
    cr_assert_eq(skJson_bool_value(skJson_array_front(ret)), false);
    cr_assert_eq(skJson_array_back(ret)->type, SK_NULL_NODE);
    cr_assert_eq(skJson_array_len(ret), 5);
    skJson_array_clear(ret);
    cr_assert_eq(skJson_array_len(ret), 0);
    cr_assert(skJson_array_push_ref(
        ret,
        "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIOMqqnkVzrm0SdG6UOoqKLsabgH5C9okWi0dh2l9GKJl"));
    cr_assert(skJson_array_push_ref(
        ret,
        "ecdsa-sha2-nistp256 "
        "AAAAE2VjZHNhLXNoYTItbmlzdHAyNTYAAAAIbmlzdHAyNTYAAABBBEmKSENjQEezOmxkZMy7opKgwFB9nkt5Y"
        "RrYMjNuG5N87uRgg6CLrbo5wAdT/y6v0mKV0U2w0WZ2YB/++Tpockg="));
    cr_assert(skJson_array_push_ref(
        ret,
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
    cr_assert_eq(skJson_array_len(ret), 3);
    cr_assert_eq(skJson_array_front(ret)->type, SK_REFERENCE_NODE);
    cr_assert_eq(skJson_array_back(ret)->type, SK_REFERENCE_NODE);
    cr_assert_eq(skJson_array_index(ret, 1)->type, SK_REFERENCE_NODE);

    skJson* node;
    int     control = 0;

    node = skJson_array_front(ret);
    cr_assert(skJson_transform_into_int(node, 15)->data.j_int == 15);
    cr_assert(node->data.j_int == 15);
    cr_assert(skJson_integer_value(node, &control) == 15);
    cr_assert(control == 0); /* No error */
    cr_assert_eq(node->type, SK_INT_NODE);
    cr_assert(skJson_parent(node));
    cr_assert(skJson_parent_type(node) == SK_ARRAY_NODE);

    /* Transform into double. */
    cr_assert_eq(skJson_transform_into_double(node, 69.69)->data.j_double, 69.69);
    cr_assert(skJson_double_value(node, &control) == 69.69);
    cr_assert(control == 0); /* No error */
    cr_assert_eq(node->type, SK_DOUBLE_NODE);
    cr_assert(skJson_parent(node));
    cr_assert(skJson_parent_type(node) == SK_ARRAY_NODE);

    /* Transform into boolean. */
    cr_assert_eq(skJson_transform_into_bool(node, false)->data.j_boolean, false);
    cr_assert(skJson_bool_value(node) == false);
    cr_assert_eq(node->type, SK_BOOL_NODE);
    cr_assert(skJson_parent(node));
    cr_assert(skJson_parent_type(node) == SK_ARRAY_NODE);

    /* Transform into reference. */
    cr_assert_str_eq(
        skJson_transform_into_ref(node, "Random bullshit")->data.j_string,
        "Random bullshit");
    cr_assert_str_eq(skJson_string_ref_unsafe(node), "Random bullshit");
    cr_assert_eq(node->type, SK_REFERENCE_NODE);
    cr_assert(skJson_parent(node));
    cr_assert(skJson_parent_type(node) == SK_ARRAY_NODE);

    /* Transform into string. */
    char* tempstr;
    cr_assert_str_eq(
        skJson_transform_into_string(node, "Random bullshit 2")->data.j_string,
        "Random bullshit 2");
    cr_assert_str_eq(skJson_string_ref_unsafe(node), "Random bullshit 2");
    cr_assert_str_eq((tempstr = skJson_string_value(node)), "Random bullshit 2");
    free(tempstr);
    cr_assert_eq(node->type, SK_STRING_NODE);
    cr_assert(skJson_parent(node));
    cr_assert(skJson_parent_type(node) == SK_ARRAY_NODE);

    /* Transform it into empty array and do array stuff. */
    cr_assert(skJson_transform_into_empty_array(node)->data.j_array != NULL);
    cr_assert(skJson_array_len(node) == 0);
    cr_assert_eq(node->type, SK_ARRAY_NODE);
    cr_assert(skJson_parent(node));
    cr_assert(skJson_array_insert_null(node, 0));
    cr_assert(skJson_array_insert_bool(node, false, 1));
    cr_assert(skJson_array_insert_double(node, 15.15, 0));
    cr_assert(skJson_array_insert_int(node, 100, 2));
    cr_assert(skJson_array_insert_ref(node, "Wow", 2));
    cr_assert(skJson_array_push_str(node, "Wow owned"));
    skJson new_element = skJson_double_new(15.5);
    cr_assert(skJson_array_push_element(node, &new_element));
    new_element = skJson_bool_new(true);
    cr_assert(skJson_array_push_element(node, &new_element));
    new_element = skJson_ref_new("Reference");
    cr_assert(skJson_array_insert_element(node, &new_element, 6));
    cr_assert(skJson_parent_type(node) == SK_ARRAY_NODE);
    cr_assert(skJson_array_len(node) == 9);

    /* Transform it into object and do object stuff. */
    cr_assert(skJson_transform_into_empty_object(node)->data.j_object != NULL);
    cr_assert(skJson_object_len(node) == 0);
    cr_assert_eq(skJson_type(node), SK_OBJECT_NODE);
    cr_assert(skJson_parent(node));
    cr_assert(skJson_parent_type(node) == SK_ARRAY_NODE);
    new_element = skJson_bool_new(true);
    cr_assert(skJson_object_insert_element(node, "key1", &new_element, 0));
    cr_assert(skJson_object_insert_bool(node, "key2", false, 1));
    cr_assert(skJson_object_insert_double(node, "key3", 15.15, 0));
    cr_assert(skJson_object_insert_int(node, "key4", 100, 2));
    cr_assert(skJson_object_insert_ref(node, "key5", "refstring", 2));
    cr_assert(skJson_object_push_string(node, "key6", "ownedstring"));
    cr_assert(skJson_object_len(node) == 6);
    cr_assert(skJson_object_index(node, 0) != NULL);
    cr_assert(skJson_object_index(node, 0)->key != NULL);
    cr_assert(skJson_object_index(node, 0)->value.type == SK_DOUBLE_NODE);
    cr_assert(skJson_object_index(node, 0)->value.data.j_double == 15.15);
    cr_assert_str_eq(skJson_object_index(node, 0)->key, "key3");
    cr_assert(skJson_object_remove(node, 0));
    cr_assert(skJson_object_len(node) == 5);
    cr_assert(skJson_object_remove(node, 2));
    cr_assert(skJson_object_len(node) == 4);
    cr_assert(skJson_object_remove(node, 4) == false);
    cr_assert(skJson_object_len(node) == 4);
    cr_assert(skJson_object_remove(node, 3));
    cr_assert(skJson_object_len(node) == 3);
    skObjTuple tuple;
    cr_assert(skJson_object_pop(node, &tuple));
    skObjTuple_drop(&tuple);
    cr_assert(skJson_object_len(node) == 2);
    cr_assert(skJson_object_pop(node, &tuple));
    skObjTuple_drop(&tuple);
    cr_assert(skJson_object_len(node) == 1);
    cr_assert(skJson_object_pop(node, &tuple));
    skObjTuple_drop(&tuple);
    cr_assert(skJson_object_len(node) == 0);
    cr_assert(skJson_object_push_int(node, "key5", 55));
    cr_assert(skJson_object_push_int(node, "key2", 22));
    cr_assert(skJson_object_push_int(node, "key4", 44));
    cr_assert(skJson_object_push_int(node, "key3", 33));
    cr_assert(skJson_object_push_int(node, "key1", 11));
    cr_assert_str_eq(skJson_object_index(node, 0)->key, "key5");
    cr_assert_str_eq(skJson_object_index(node, 1)->key, "key2");
    cr_assert_str_eq(skJson_object_index(node, 2)->key, "key4");
    cr_assert_str_eq(skJson_object_index(node, 3)->key, "key3");
    cr_assert_str_eq(skJson_object_index(node, 4)->key, "key1");
    cr_assert_not(skJson_object_is_sorted(node));
    cr_assert(skJson_object_sort(node));
    cr_assert(skJson_object_is_sorted(node));
    cr_assert_str_eq(skJson_object_index(node, 0)->key, "key1");
    cr_assert_str_eq(skJson_object_index(node, 1)->key, "key2");
    cr_assert_str_eq(skJson_object_index(node, 2)->key, "key3");
    cr_assert_str_eq(skJson_object_index(node, 3)->key, "key4");
    cr_assert_str_eq(skJson_object_index(node, 4)->key, "key5");
    cr_assert(skJson_object_sort_by(node, (CmpFn) cmp_tuples_descending));
    cr_assert_str_eq(skJson_object_index(node, 0)->key, "key5");
    cr_assert_str_eq(skJson_object_index(node, 1)->key, "key4");
    cr_assert_str_eq(skJson_object_index(node, 2)->key, "key3");
    cr_assert_str_eq(skJson_object_index(node, 3)->key, "key2");
    cr_assert_str_eq(skJson_object_index(node, 4)->key, "key1");
    skJson_drop(&skJson_object_index(node, 4)->value);
    cr_assert(skJson_object_index(node, 4)->value.type == SK_NULL_NODE);
    cr_assert(skJson_object_is_sorted(node) == false);
    cr_assert(skJson_object_is_sorted_by(node, (CmpFn) cmp_tuples_descending));
    cr_assert_str_eq(
        skJson_object_index(node, 2)->key,
        skJson_object_index_by_key(node, "key3", false)->key);

    /* Serialize the json */
    unsigned char* out;
    out = skJson_serialize(&json_final);
    cr_assert(out != NULL);
    free(out);
}
