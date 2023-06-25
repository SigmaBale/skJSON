// clang-format off
#include "sktoken.h"
#include "skscanner.h"
#include "sknode.h"
#include "skparser.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
// clang-format on

#define SK_EMPTY_STRING ""

skJsonString skJsonString_new_internal(skToken token);
skJsonNode*  skparse_json_object(skScanner* scanner, skJsonNode* parent);
skJsonNode*  skparse_json_array(skScanner* scanner, skJsonNode* parent);
skJsonNode*  skparse_json_string(skScanner* scanner, skJsonNode* parent);
skJsonNode*  skparse_json_number(skScanner* scanner, skJsonNode* parent);
skJsonNode*  skparse_json_bool(skScanner* scanner, skJsonNode* parent);
skJsonNode*  skparse_json_null(skScanner* scanner, skJsonNode* parent);

/// TODO: Implement public interface around this struct
struct skJson {
    skJsonNode* root;
};

skJson*
sk_json_new(void* buff, size_t bufsize)
{
    if(buff == NULL) {
        return NULL;
    }

    skJsonNode* root;
    skScanner*  scanner = skScanner_new(buff, bufsize);

    if(scanner == NULL) {
        return NULL;
    }

    skJson* json = malloc(sizeof(skJson));

    if(json == NULL) {
        free(scanner);
        PRINT_OOM_ERR;
        return NULL;
    }

    /// Construct the parse tree
    root = skJsonNode_new(scanner, NULL);

    if(root == NULL) {
        free(scanner);
        free(json);
        return NULL;
    }

    json->root = root;

    return json;
}

skJsonNode*
skJsonNode_new(skScanner* scanner, skJsonNode* parent)
{
    /// Peek at current token without advancing
    skToken next = skScanner_peek(scanner);

    switch(next.type) {
        case SK_LCURLY:
            return skparse_json_object(scanner, parent);
        case SK_LBRACK:
            return skparse_json_array(scanner, parent);
        case SK_STRING:
            return skparse_json_string(scanner, parent);
        case SK_HYPHEN:
        case SK_ZERO:
        case SK_DIGIT:
            return skparse_json_number(scanner, parent);
        case SK_FALSE:
        case SK_TRUE:
            return skparse_json_bool(scanner, parent);
        case SK_NULL:
            return skparse_json_null(scanner, parent);
        case SK_INVALID:
        default:
            printf("Just got invalid token -> ");
            skToken_print(next);
            return skJsonError_new(
                "Invalid syntax/token while parsing",
                parent);
    }
}

skJsonNode*
skparse_json_object(skScanner* scanner, skJsonNode* parent)
{
    /// Error indicator
    bool         err = false;
    /// First iteration flag
    bool         start = true;
    skJsonString key   = NULL;
    skJsonNode*  value = NULL;

    /// Empty Json Object
    skJsonNode* object_node = skJsonObject_new(parent);
    null_check_with_ret(object_node, NULL);
    /// Lookup table
    skHashTable* table = object_node->data.j_object;
    /// Take the first token after '{'
    skToken      token = skScanner_next(scanner);
    skScanner_skip(scanner, 2, SK_WS, SK_NL);
    /// If next token is '}', return empty object '{}'
    if(skScanner_peek(scanner).type == SK_RCURLY) {
        skScanner_next(scanner);
        return object_node;
    }

    do {
        if(!start) {
            skScanner_next(scanner);
            skScanner_skip(scanner, 2, SK_WS, SK_NL);
        } else {
            start = false;
        }

        if((token = skScanner_peek(scanner)).type == SK_STRING) {
            key = skJsonString_new_internal(token);
            null_check_with_ret(key, NULL);
            skScanner_next(scanner);

            skScanner_skip(scanner, 1, SK_WS);

            if(skScanner_peek(scanner).type != SK_COLON) {
                err = true;
                break;
            } else {
                skScanner_next(scanner);
            }

            skScanner_skip(scanner, 1, SK_WS);
            value = skJsonNode_new(scanner, NULL);
            null_check_with_ret(value, NULL);

            if(value->type == SK_ERROR_NODE) {
                err = true;
                break;
            }
        } else {
            /// If token is not '}' or 'String', then error
            err = true;
            break;
        }

        size_t tlen = skHashTable_len(table);
        if(__glibc_unlikely(!skHashTable_insert(table, key, value))) {
            return NULL;
        }
        assert(tlen + 1 == skHashTable_len(table));

        skScanner_skip(scanner, 2, SK_WS, SK_NL);

    } while((token = skScanner_peek(scanner)).type == SK_COMMA);

    /// Return ErrorNode if err occured or object isn't enclosed by '}'.
    if(err || token.type != SK_RCURLY) {
        /// Cleanup the Members of the Object before returning error
        skHashTable_drop(table);
        /// Return error
        return skJsonError_new("failed parsing json object", parent);
    }

    /// Return valid Json Object.
    return object_node;
}

skJsonNode*
skparse_json_array(skScanner* scanner, skJsonNode* parent)
{
    skToken     token = skScanner_next(scanner);
    /// Error flag
    bool        err = false;
    /// Start of iteration flag
    bool        start = true;
    skJsonNode* temp  = NULL;

    /// Json Array
    skJsonNode* array_node = skJsonArray_new(parent);
    null_check_with_ret(array_node, NULL);
    /// Internal skVec
    skVec* array = array_node->data.j_array;
    skScanner_skip(scanner, 2, SK_WS, SK_NL);

    /// If next token ']' return empty array '[]'
    if(token.type == SK_RBRACK) {
        return array_node;
    }

    do {
        if(!start) {
            skScanner_next(scanner);
            skScanner_skip(scanner, 2, SK_WS, SK_NL);
        } else {
            start = false;
        }

        temp = skJsonNode_new(scanner, array_node);

        if(temp->type == SK_ERROR_NODE || __glibc_unlikely(is_null(temp))) {
            if(__glibc_unlikely(is_null(temp))) {
                skVec_drop(array, (FreeFn) skJsonNode_drop);
                free(array_node);
                return NULL;
            }
            /// If node parsing errored set err
            err = true;
            break;
        }

        temp->index = array->len;
        /// Add newly parsed node to the Json Array
        if(__glibc_unlikely(!skVec_push(array, temp))) {
            skVec_drop(array, (FreeFn) skJsonNode_drop);
            free(temp);
            free(array_node);
            return NULL;
        }
        /// Free the temporary node
        free(temp);
        skScanner_skip(scanner, 2, SK_WS, SK_NL);

    } while((token = skScanner_peek(scanner)).type == SK_COMMA);
    /// Return ErrorNode if err occured or array isn't enclosed by ']'
    if(err || token.type != SK_RBRACK) {
        /// Cleanup the Nodes of the Array before returning error
        skVec_drop(array, (FreeFn) skJsonNode_drop);

        /// Skip this depth where error occured, so we can
        /// continue parsing the rest of the file
        /// TODO: Remove this
        skCharIter_depth_above(&scanner->iter);

        /// Return error
        return skJsonError_new("failed parsing json array", parent);
    } else {
        /// If we reached the end '}' with no errors,
        /// advance the scanner
        skScanner_next(scanner);
    }

    /// Return valid Json Array
    return array_node;
}

/// Checks the validity of a string and allocates
/// storage for it if the json string is valid
skJsonString
skJsonString_new_internal(skToken token)
{
    /// Number of hexadecimal code points for UTF-16 encoding
    static const unsigned char UNIC_HEX_UTF16_CODES = 4;

    /// If string is empty "" return the empty string...
    if(token.lexeme.len == 0) {
        return SK_EMPTY_STRING;
    }

    int  c;
    bool hex = false;

    skCharIter iterator = skCharIter_from_slice(&token.lexeme);

    iterator.state.in_jstring = true;
    while((c = skCharIter_next(&iterator)) != EOF) {
        if(c < 020) {
            return NULL;
        }

        /// Check if the hex value is valid
        for(int count = 0; hex && count < UNIC_HEX_UTF16_CODES; count++) {
            if(!isxdigit(c)) {
                return NULL;
            }
        }

        hex = false;

        if(c == '\\') {
            c = skCharIter_next(&iterator);

            switch(c) {
                case '\\':
                case '"':
                case '/':
                case 'b':
                case 'f':
                case 'n':
                case 'r':
                case 't':
                    break;
                case 'u':
                    hex = true;
                    break;
                default:
                    return NULL;
                    break;
            }
        }
    }

    size_t       bytes   = token.lexeme.len;
    skJsonString jstring = malloc(bytes + 1); /// +1 for null terminator
    null_check_with_err_and_ret(jstring, PRINT_OOM_ERR, NULL);

    /// Copy over 'bytes' amount into str
    jstring = strncpy(jstring, token.lexeme.ptr, bytes);
    /// Null terminate the str (that's why we added +1 to malloc)
    jstring[bytes] = '\0';
#ifdef SK_DBUG
    assert(strlen(jstring) == bytes);
#endif
    iterator.state.in_jstring = false;
    return jstring;
}

skJsonNode*
skparse_json_string(skScanner* scanner, skJsonNode* parent)
{
    /// Get the SK_STRING token
    skToken      token   = skScanner_peek(scanner);
    skJsonString jstring = NULL;

    /// Parse the Json String
    if((jstring = skJsonString_new_internal(token)) == NULL) {
        skCharIter_depth_above(&scanner->iter);
        /// Parsing failed, return error
        return skJsonError_new("failed to parse Json String", parent);
    }

    skScanner_next(scanner);

    /// Return valid Json String (null terminated)
    return skJsonString_new(jstring, parent);
}

skJsonNode*
skparse_json_number(skScanner* scanner, skJsonNode* parent)
{
    /// Fetch the first token (either a digit, zero or hyphen)
    skToken token = skScanner_peek(scanner);
    /// Store the start of the Json Number
    char*   start = skCharIter_next_address(&scanner->iter) - 1;

    /// Flags for control flow and validity check
    bool negative = false;
    bool integer  = false;
    bool fraction = false;

    /// If first token is HYPHEN
    if(token.type == SK_HYPHEN) {
        /// We have a 'negative' number
        negative = true;
        token    = skScanner_next(scanner);
    }

    /// If we have a digit (1-9) or zero (0) parse the integer part
    if(token.type == SK_DIGIT || token.type == SK_ZERO) {
        /// We have integer
        integer = true;

        /// Integer part is zero
        if(token.type == SK_ZERO) {
            token = skScanner_next(scanner);
            if(token.type == SK_DIGIT || token.type == SK_ZERO) {
                goto jmp_err;
            }
        } else {
            skScanner_skip(scanner, 2, SK_DIGIT, SK_ZERO);
        }
    } else if(negative) {
        /// If we didn't find integer part but we have a HYPHEN
        /// aka minus sign, that is invalid number, goto error
        goto jmp_err;
    }

    /// Let's check if there is fraction
    token = skScanner_peek(scanner);

    if(token.type == SK_DOT) {
        /// If we have a decimal point but no
        /// integer part, that is invalid json number
        if(!integer) {
            goto jmp_err;
        } else {
            skScanner_next(scanner);
        }

        if((token = skScanner_peek(scanner)).type == SK_DIGIT
           || token.type == SK_ZERO)
        {
            /// Set fraction as valid
            fraction = true;
            skScanner_skip(scanner, 2, SK_DIGIT, SK_ZERO);
        }

        /// If fraction is invalid, goto error
        if(!fraction) {
            goto jmp_err;
        }
    }

    /// Let's check if there is exponent
    if(skScanner_peek(scanner).type == SK_EXP) {
        /// If there is no fractional part, then we can't have
        /// exponent part, goto error
        if(!fraction) {
            goto jmp_err;
        }

        if((token = skScanner_next(scanner)).type != SK_HYPHEN
           && token.type != SK_PLUS)
        {
            /// If exponent doesn't have a sign it is invalid, goto error
            goto jmp_err;
        }

        if((token = skScanner_next(scanner)).type == SK_DIGIT
           || token.type == SK_ZERO)
        {
            /// Exponent is valid, skip the rest of the digits/zeroes
            skScanner_skip(scanner, 2, SK_DIGIT, SK_ZERO);
        } else {
            /// Exponent is invalid, it contains no digits, goto error
            goto jmp_err;
        }
    }

    /// Set the end of the token stream
    char* end = skCharIter_next_address(&scanner->iter) - 1;

    /// Convert the stream into number (double)
    skJsonDouble number = strtod(start, &end);

    /// If during conversion overflow occured, warn the user
    /// TODO: More direct error propagation to user
    if(errno == ERANGE) {
        fprintf(stderr, "warning: Json Number overflow detected\n");
    }

    if(fraction) {
        return skJsonDouble_new(number, parent);
    } else {
        return skJsonInteger_new((skJsonInteger) number, parent);
    }

jmp_err:
    /// Skip entire json object/array to continue parsing rest of the file
    /// TODO: Remove this, we will always just propagate error and return
    /// error node instead
    skCharIter_depth_above(&scanner->iter);
    /// Finally return the error node
    return skJsonError_new("failed to parse Json Number", parent);
}

void
print_node(skJsonNode* node)
{
    if(node == NULL) {
        printf("NO PARENT\n");
        return;
    }
    switch(node->type) {
        case SK_ERROR_NODE:
            printf("ERROR NODE: %s\n", node->data.j_err);
            break;
        case SK_OBJECT_NODE:
            printf("OBJECT NODE\n");
            break;
        case SK_ARRAY_NODE:
            printf("ARRAY NODE\n");
            break;
        case SK_STRING_NODE:
            printf("STRING NODE -> '%s'\n", node->data.j_string);
            break;
        case SK_EMPTYSTRING_NODE:
            printf("EMPTYSTRING NODE\n");
            break;
        case SK_INT_NODE:
            printf("INT NODE: %ld\n", node->data.j_int);
            break;
        case SK_DOUBLE_NODE:
            printf("DOUBLE NODE: %lf\n", node->data.j_double);
            break;
        case SK_BOOL_NODE:
            printf(
                "BOOL NODE: %s\n",
                (node->data.j_boolean) ? "true" : "false");
            break;
        case SK_NULL_NODE:
            printf("NULL NODE\n");
            break;
        case SK_MEMBER_NODE:
            printf("MEMBER NODE: key: %s, value: ", node->data.j_member->key);
            print_node(node->data.j_member->value);
            break;
    }
}

skJsonNode*
skparse_json_bool(skScanner* scanner, skJsonNode* parent)
{
    /// Fetch the bool token
    skToken token = skScanner_peek(scanner);

    if(token.type == SK_TRUE) {
        skScanner_next(scanner);
        /// token is SK_TRUE, create True BoolNode
        return skJsonBool_new(true, parent);
    } else {
        skScanner_next(scanner);
        /// token is SK_FALSE, create False BoolNode
        return skJsonBool_new(false, parent);
    }
}

skJsonNode*
skparse_json_null(skScanner* scanner, skJsonNode* parent)
{
    skScanner_next(scanner);
    return skJsonNull_new(parent);
}
