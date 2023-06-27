#ifdef SK_DBUG
#include <assert.h>
#endif
#include "skerror.h"
#include "skparser.h"
#include "skutils.h"
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

skJsonString skJsonString_new_internal(skToken token);
skJsonNode*  skparse_json_object(skScanner* scanner, skJsonNode* parent);
skJsonNode*  skparse_json_array(skScanner* scanner, skJsonNode* parent);
skJsonNode*  skparse_json_string(skScanner* scanner, skJsonNode* parent);
skJsonNode*  skparse_json_number(skScanner* scanner, skJsonNode* parent);
skJsonNode*  skparse_json_bool(skScanner* scanner, skJsonNode* parent);
skJsonNode*  skparse_json_null(skScanner* scanner, skJsonNode* parent);

skJsonNode*
skJsonNode_new(skScanner* scanner, skJsonNode* parent)
{
    skToken next;

    /* Peek at current token without advancing */
    next = skScanner_peek(scanner);
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
            return skJsonError_new(
                "Invalid syntax/token while parsing",
                scanner->iter.state,
                parent);
    }
}

skJsonNode*
skparse_json_object(skScanner* scanner, skJsonNode* parent)
{
    skJsonNode*  object_node;
    skHashTable* table;
    skJsonString key;
    skJsonNode*  value;
    skToken      token;
    bool         err;   /* Error flag */
    bool         start; /* First iteration flag */

    err   = false;
    start = true;

    object_node = skJsonObject_new(parent);
    if(is_null(object_node)) {
        return NULL;
    }

    /* Table components */
    table = object_node->data.j_object;
    key   = NULL;
    value = NULL;

    /* Take the first token after '{' */
    token = skScanner_next(scanner);
    skScanner_skip(scanner, 2, SK_WS, SK_NL);

    /* If next token is '}', return empty object '{}' */
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

            /* Parse the key */
            key = skJsonString_new_internal(token);
            if(is_null(key)) {
                return NULL;
            } else {
                skScanner_next(scanner);
            }

            skScanner_skip(scanner, 1, SK_WS);

            if(skScanner_peek(scanner).type != SK_COLON) {
                free(key);
                err = true;
                break;
            } else {
                skScanner_next(scanner);
            }

            skScanner_skip(scanner, 1, SK_WS);

            /* Parse the value */
            value = skJsonNode_new(scanner, NULL);

            if(is_null(value)) {
                free(key);
                skJsonNode_drop(object_node);
                return NULL;
            }

            if(value->type == SK_ERROR_NODE) {
                free(key);
                err = true;
                break;
            }
        } else {
            err = true;
            break;
        }

        if(!skHashTable_insert(table, key, value)) {
            free(key);
            skJsonNode_drop(value);
            skJsonNode_drop(object_node);
            return NULL;
        }

        skScanner_skip(scanner, 2, SK_WS, SK_NL);

    } while((token = skScanner_peek(scanner)).type == SK_COMMA);

    /* Return ErrorNode if err occured or object isn't enclosed by '}'. */
    if(err || token.type != SK_RCURLY) {
        skJsonNode_drop(object_node);
        /* If 'value' was the error node return it, otherwise
         * parsing this object was the reason of error. */
        if(value->type == SK_ERROR_NODE) {
            return value;
        } else {
            return skJsonError_new(
                "failed parsing json object",
                scanner->iter.state,
                parent);
        }
    } else {
        skScanner_next(scanner);
        skScanner_skip(scanner, 2, SK_WS, SK_NL);
    }
    return object_node;
}

skJsonNode*
skparse_json_array(skScanner* scanner, skJsonNode* parent)
{
    skJsonNode* temp;
    skJsonNode* array_node;
    skVec*      array;
    skToken     token;
    bool        err;      /* Error flag */
    bool        start;    /* First iteration flag */
    bool        temp_err; /* Temp value error flag */

    start    = true;
    err      = false;
    temp_err = false;
    token    = skScanner_next(scanner);
    temp     = NULL;

    array_node = skJsonArray_new(parent);
    if(is_null(array_node)) {
        return NULL;
    }

    array = array_node->data.j_array;
    skScanner_skip(scanner, 2, SK_WS, SK_NL);

    /* If next token ']' return empty array '[]' */
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

        /* Parse the array node */
        temp = skJsonNode_new(scanner, array_node);
        if(is_null(temp)) {
            skJsonNode_drop(array_node);
            return NULL;
        }

        if(temp->type == SK_ERROR_NODE) {
            temp_err = err = true;
            break;
        }

        temp->index = skVec_len(array);

        /* Add newly parsed node to the Json Array */
        if(!skVec_push(array, temp)) {
            skJsonNode_drop(temp);
            skJsonNode_drop(array_node);
            return NULL;
        }

        free(temp); /* Free the wrapper but not the underlying objects */
        skScanner_skip(scanner, 2, SK_WS, SK_NL);

    } while((token = skScanner_peek(scanner)).type == SK_COMMA);

    /* Return ErrorNode if err occured or array isn't enclosed by ']' */
    if(err || token.type != SK_RBRACK) {
        skJsonNode_drop(array_node);
        /* If 'temp' node is error node return it, otherwise
         * error occured in parsing this Json Array. */
        if(temp_err) {
            return temp;
        } else {
            return skJsonError_new(
                "failed parsing json array",
                scanner->iter.state,
                parent);
        }
    } else {
        skScanner_next(scanner);
        skScanner_skip(scanner, 2, SK_WS, SK_NL);
    }

    return array_node;
}

/* Checks the validity of a string and allocates
 * storage for it if the json string is valid */
skJsonString
skJsonString_new_internal(skToken token)
{
    /* Number of hexadecimal code points for UTF-16 encoding */
    static const unsigned char UNIC_HEX_UTF16_CODES = 4;

    int          c, count;
    bool         hex;
    size_t       bytes;
    skCharIter   iterator;
    skJsonString jstring;

    hex      = false;
    iterator = skCharIter_from_slice(&token.lexeme);

    /* If string is empty "" return the empty string... */
    if(token.lexeme.len == 0) {
        goto alloc_str;
    }

    while((c = skCharIter_next(&iterator)) != EOF) {
        if(c < 020) {
            return NULL;
        }

        /* Check if the hex value is valid */
        for(count = 0; hex && count < UNIC_HEX_UTF16_CODES; count++) {
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

alloc_str:
    bytes = token.lexeme.len;

    jstring = malloc(bytes + 1); /* +1 for null terminator */
    if(is_null(jstring)) {
        THROW_ERR(OutOfMemory);
        return NULL;
    }

    jstring        = strncpy(jstring, token.lexeme.ptr, bytes);
    jstring[bytes] = '\0';
    return jstring;
}

skJsonNode*
skparse_json_string(skScanner* scanner, skJsonNode* parent)
{
    skToken      token;
    skJsonString jstring;

    /* Get the SK_STRING token */
    token   = skScanner_peek(scanner);
    jstring = skJsonString_new_internal(token);

    if(is_null(jstring)) {
        return skJsonError_new("failed parsing Json String", scanner->iter.state, parent);
    } else {
        skScanner_next(scanner);
    }

    /* Return valid Json String (null terminated) */
    return skJsonString_new(jstring, parent);
}

skJsonNode*
skparse_json_number(skScanner* scanner, skJsonNode* parent)
{
    skJsonInteger integ;
    skJsonDouble  dbl;
    skToken       token;
    char*         start;
    char*         end;
    bool          negative;
    bool          integer;
    bool          fraction;

    /* Flags for control flow and validity check */
    negative = false;
    integer  = false;
    fraction = false;

    /* Fetch the first token (either a digit, zero or hyphen) */
    token = skScanner_peek(scanner);
    /* Start of the number stream */
    start = token.lexeme.ptr;

    /* If first token is HYPHEN, we have a 'negative' number */
    if(token.type == SK_HYPHEN) {
        negative = true;
        token    = skScanner_next(scanner);
    }

    /* If we have a digit (1-9) or zero (0) we have a integer part */
    if(token.type == SK_DIGIT || token.type == SK_ZERO) {
        integer = true;

        /* Integer part is zero */
        if(token.type == SK_ZERO) {
            token = skScanner_next(scanner);
            if(token.type == SK_DIGIT || token.type == SK_ZERO) {
                goto jmp_err;
            }
        } else {
            skScanner_skip(scanner, 2, SK_DIGIT, SK_ZERO);
        }
    } else if(negative) {
        /* Number can't be negative without integer part */
        goto jmp_err;
    }

    /* Let's check if there is fraction */
    if(skScanner_peek(scanner).type == SK_DOT) {
        /* Number with decimal dot must have integer part */
        if(!integer) {
            goto jmp_err;
        } else {
            skScanner_next(scanner);
        }

        if((token = skScanner_peek(scanner)).type == SK_DIGIT || token.type == SK_ZERO) {
            /* Fraction contains some digits, it is valid */
            fraction = true;
            skScanner_skip(scanner, 2, SK_DIGIT, SK_ZERO);
        }

        /* If fraction is invalid, goto error */
        if(!fraction) {
            goto jmp_err;
        }
    }

    /* Let's check if there is exponent */
    if(skScanner_peek(scanner).type == SK_EXP) {
        /* If there is no fractional part, then we can't have exponent part */
        if(!fraction) {
            goto jmp_err;
        }

        /* If exponent doesn't have a sign it is invalid, goto error */
        if((token = skScanner_next(scanner)).type != SK_HYPHEN && token.type != SK_PLUS) {
            goto jmp_err;
        }

        /* Exponent must contain digit/s to be valid */
        if((token = skScanner_next(scanner)).type == SK_DIGIT || token.type == SK_ZERO) {
            skScanner_skip(scanner, 2, SK_DIGIT, SK_ZERO);
        } else {
            goto jmp_err;
        }
    }

    /* Set the end of the token stream */
    end = skCharIter_next_address(&scanner->iter) - 1;

    if(fraction) {
        dbl = strtod(start, &end);
        if(errno == ERANGE) {
            THROW_WARN(OverflowDetected, scanner);
        }
        return skJsonDouble_new(dbl, parent);
    } else {
        integ = strtol(start, &end, 10);
        if(errno == ERANGE) {
            THROW_WARN(OverflowDetected, scanner);
        }
        return skJsonInteger_new(integ, parent);
    }

jmp_err:
    return skJsonError_new("failed to parse Json Number", scanner->iter.state, parent);
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
            printf("OBJECT NODE -> len: %lu\n", skHashTable_len(node->data.j_object));
            break;
        case SK_ARRAY_NODE:
            printf("ARRAY NODE -> len: %lu\n", skVec_len(node->data.j_array));
            break;
        case SK_STRING_NODE:
            printf("STRING NODE -> '%s'\n", node->data.j_string);
            break;
        case SK_INT_NODE:
            printf("INT NODE: %ld\n", node->data.j_int);
            break;
        case SK_DOUBLE_NODE:
            printf("DOUBLE NODE: %f\n", node->data.j_double);
            break;
        case SK_BOOL_NODE:
            printf("BOOL NODE: %s\n", (node->data.j_boolean) ? "true" : "false");
            break;
        case SK_NULL_NODE:
            printf("NULL NODE\n");
            break;
    }
}

skJsonNode*
skparse_json_bool(skScanner* scanner, skJsonNode* parent)
{
    skToken token;

    /* Fetch the bool token */
    token = skScanner_peek(scanner);

    if(token.type == SK_TRUE) {
        skScanner_next(scanner);
        return skJsonBool_new(true, parent);
    } else {
        skScanner_next(scanner);
        return skJsonBool_new(false, parent);
    }
}

skJsonNode*
skparse_json_null(skScanner* scanner, skJsonNode* parent)
{
    skScanner_next(scanner);
    return skJsonNull_new(parent);
}
