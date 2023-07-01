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

skJsonString skJsonString_new_internal(skStrSlice* slice);
skJsonNode*  skparse_json_object(skScanner* scanner, skJsonNode* parent);
skJsonNode*  skparse_json_array(skScanner* scanner, skJsonNode* parent);
skJsonNode*  skparse_json_string(skScanner* scanner, skJsonNode* parent);
skJsonNode*  skparse_json_number(skScanner* scanner, skJsonNode* parent);
skJsonNode*  skparse_json_bool(skScanner* scanner, skJsonNode* parent);
skJsonNode*  skparse_json_null(skScanner* scanner, skJsonNode* parent);

skJsonNode*
skJsonNode_parse(skScanner* scanner, skJsonNode* parent)
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
            return ErrorNode_new(
                "Invalid syntax/token while parsing",
                scanner->iter.state,
                parent);
    }
}

skJsonNode*
skparse_json_object(skScanner* scanner, skJsonNode* parent)
{
    skJsonNode*   object_node;
    skObjectTuple tuple;
    skJsonNode*   value;
    skToken       token;
    skVec*        table;
    bool          err;   /* Error flag */
    bool          start; /* First iteration flag */

    err   = false;
    start = true;

    object_node = ObjectNode_new(parent);
    if(is_null(object_node)) {
        return NULL;
    }

    table = object_node->data.j_object;

    /* Take the first token after '{' */
    token = skScanner_next(scanner);
    skScanner_skip(scanner, 2, SK_WS, SK_NL);

    /* If next token is '}', return empty object '{}' */
    if(skScanner_peek(scanner).type == SK_RCURLY) {
        skScanner_next(scanner);
        return object_node;
    }

    do {
        value = NULL;

        if(!start) {
            skScanner_next(scanner);
            skScanner_skip(scanner, 2, SK_WS, SK_NL);
        } else {
            start = false;
        }

        if((token = skScanner_peek(scanner)).type == SK_STRING) {
            /* Parse the key */
            tuple.key = skJsonString_new_internal(&token.lexeme);
            if(is_null(tuple.key)) {
                return NULL;
            } else {
                skScanner_next(scanner);
            }

            skScanner_skip(scanner, 1, SK_WS);

            /* We must have ':' */
            if(skScanner_peek(scanner).type != SK_COLON) {
                free(tuple.key);
                err = true;
                break;
            } else {
                skScanner_next(scanner);
            }

            skScanner_skip(scanner, 1, SK_WS);

            /* Parse the value */
            value = skJsonNode_parse(scanner, object_node);

            if(is_null(value)) {
                free(tuple.key);
                skJsonNode_drop(object_node);
                return NULL;
            }

            if(value->type == SK_ERROR_NODE) {
                free(tuple.key);
                err = true;
                break;
            }
#ifdef SK_DBUG
            assert(value->parent->type == SK_OBJECT_NODE);
#endif
            value->index = skVec_len(table);
            memcpy(&tuple.value, value, sizeof(skJsonNode));
            tuple.value.index = skVec_len(table);

            /* Free the wrapper */
            free(value);

        } else {
            err = true;
            break;
        }

        if(!skVec_push(table, &tuple)) {
            skObjectTuple_drop(&tuple);
            skJsonNode_drop(object_node);
            return NULL;
        }

        skScanner_skip(scanner, 2, SK_WS, SK_NL);

    } while((token = skScanner_peek(scanner)).type == SK_COMMA);

    /* Return ErrorNode if err occured or object isn't enclosed by '}'. */
    if(err || token.type != SK_RCURLY) {
        skJsonNode_drop(object_node);
        /* If 'value' was the error node return it, otherwise
         * error happened parsing this object */
        if(is_some(value)) {
#ifdef SK_DBUG
            assert(value->type == SK_ERROR_NODE);
#endif
            return value;
        } else {
            return ErrorNode_new("failed parsing json object", scanner->iter.state, parent);
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

    array_node = ArrayNode_new(parent);
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

        /* Parse the array node/value */
        temp = skJsonNode_parse(scanner, array_node);
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
            return ErrorNode_new("failed parsing json array", scanner->iter.state, parent);
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
skJsonString_new_internal(skStrSlice* slice)
{
    size_t       bytes;
    skJsonString jstring;

    if(slice->len != 0 && !skJsonString_isvalid(slice)) {
        return NULL;
    }

    bytes = slice->len + sizeof("");
    if(is_null(jstring = malloc(bytes))) {
        THROW_ERR(OutOfMemory);
        return NULL;
    }

    jstring            = strncpy(jstring, slice->ptr, slice->len);
    jstring[bytes - 1] = '\0';
    return jstring;
}

bool
skJsonString_isvalid(const skStrSlice* slice)
{
    /* Number of hexadecimal code points for UTF-16 encoding */
    static const unsigned char UNIC_HEX_UTF16_CODES = 4;

    skCharIter iterator;
    int        c, count;
    bool       hex;

    hex      = false;
    iterator = skCharIter_from_slice(discard_const(slice));

    while((c = skCharIter_next(&iterator)) != EOF) {
        if(c < 020) {
            return false;
        }

        /* Check if the hex value is valid if flag is set */
        for(count = 0; hex && count < UNIC_HEX_UTF16_CODES; count++) {
            if(!isxdigit(c)) {
                return false;
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
                    return false;
                    break;
            }
        }
    }

    return true;
}

skJsonNode*
skparse_json_string(skScanner* scanner, skJsonNode* parent)
{
    skToken      token;
    skJsonString jstring;
    skJsonNode*  string_node;

    /* Get the SK_STRING token */
    token   = skScanner_peek(scanner);
    jstring = skJsonString_new_internal(&token.lexeme);

    if(is_null(jstring)) {
        goto err; /* Maybe parse error maybe out of memory */
    } else {
        skScanner_next(scanner);
    }

    if(is_null(string_node = RawNode_new(SK_STRING_NODE, parent))) {
        return NULL;
    } else {
        string_node->data.j_string = jstring;
    }

    /* Return valid Json String (null terminated) */
    return string_node;

err:
    /* Return NULL if out of memory or error node if parse error */
    return ErrorNode_new("failed parsing Json String", scanner->iter.state, parent);
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
        return DoubleNode_new(dbl, parent);
    } else {
        integ = strtol(start, &end, 10);
        if(errno == ERANGE) {
            THROW_WARN(OverflowDetected, scanner);
        }
        return IntNode_new(integ, parent);
    }

jmp_err:
    return ErrorNode_new("failed to parse Json Number", scanner->iter.state, parent);
}

skJsonNode*
skparse_json_bool(skScanner* scanner, skJsonNode* parent)
{
    skToken token;

    /* Fetch the bool token */
    token = skScanner_peek(scanner);

    if(token.type == SK_TRUE) {
        skScanner_next(scanner);
        return BoolNode_new(true, parent);
    } else {
        skScanner_next(scanner);
        return BoolNode_new(false, parent);
    }
}

skJsonNode*
skparse_json_null(skScanner* scanner, skJsonNode* parent)
{
    skScanner_next(scanner);
    return RawNode_new(SK_NULL_NODE, parent);
}
