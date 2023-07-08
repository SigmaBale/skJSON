#include "sktypes.h"
#ifdef SK_DBUG
#include <assert.h>
#endif
#include "skerror.h"
#include "skparser.h"
#include "skutils.h"
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define set_none(node) (node).type = SK_NONE_NODE;

skJsonString skJsonString_new_internal(skScanner* scanner, skJson* err_node);
skJson       skparse_json_object(skScanner* scanner, skJson* parent);
skJson       skparse_json_array(skScanner* scanner, skJson* parent);
skJson       skparse_json_string(skScanner* scanner, skJson* parent);
skJson       skparse_json_number(skScanner* scanner, skJson* parent);
skJson       skparse_json_bool(skScanner* scanner, skJson* parent);
skJson       skparse_json_null(skScanner* scanner, skJson* parent);

skJson skJsonNode_parse(skScanner* scanner, skJson* parent)
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

skJson skparse_json_object(skScanner* scanner, skJson* parent)
{
    skJson     object_node;
    skJson     value;
    skJson     err_node;
    skObjTuple tuple;
    char*      key;
    skToken    token;
    bool       err;       /* Error flag */
    bool       parse_err; /* Parse err flag */
    bool       start;     /* First iteration flag */

    parse_err = false;
    err       = false;
    start     = true;

    err_node.type = SK_NONE_NODE;
    object_node   = ObjectNode_new(parent);
    /* Return immediately if allocation failed. */
    if(object_node.type == SK_NONE_NODE) {
        return object_node;
    }

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

            key = skJsonString_new_internal(scanner, &err_node);

            if(err_node.type == SK_ERROR_NODE) {
                err = parse_err = true;
                break;
            } else if(is_null(key)) {
                skJsonNode_drop(&object_node);
                set_none(object_node);
                return object_node;
            }

            skScanner_next(scanner);
            skScanner_skip(scanner, 1, SK_WS);

            if(skScanner_peek(scanner).type != SK_COLON) {
                free(key);
                err = true;
                break;
            }

            skScanner_next(scanner);
            skScanner_skip(scanner, 1, SK_WS);

            value = skJsonNode_parse(scanner, &object_node);

            if(value.type == SK_NONE_NODE) {
                free(key);
                skJsonNode_drop(&object_node);
                set_none(object_node);
                return object_node;
            } else if(value.type == SK_ERROR_NODE) {
                free(key);
                parse_err = err = true;
                err_node        = value;
                break;
            }

            tuple.key   = key;
            tuple.value = value;
#ifdef SK_DBUG
            assert(tuple.value.parent_arena.ptr == object_node.data.j_object);
            assert(tuple.value.parent_arena.type == SK_OBJECT_NODE);
#endif
        } else {
            err = true;
            break;
        }

        if(!skVec_push(object_node.data.j_object, &tuple)) {
            skObjTuple_drop(&tuple);
            skJsonNode_drop(&object_node);
            set_none(object_node);
            return object_node;
        }

        skScanner_skip(scanner, 2, SK_WS, SK_NL);

    } while((token = skScanner_peek(scanner)).type == SK_COMMA);

    if(err || token.type != SK_RCURLY) {
        skJsonNode_drop(&object_node);
        if(parse_err) {
            return err_node;
        } else {
            return ErrorNode_new("failed parsing Json Object", scanner->iter.state, parent);
        }
    }

    skScanner_next(scanner);
    skScanner_skip(scanner, 2, SK_WS, SK_NL);
#ifdef SK_DBUG
    assert(is_some(object_node.data.j_object));
    assert(skVec_len(object_node.data.j_object) > 0);
#endif
    return object_node;
}

skJson skparse_json_array(skScanner* scanner, skJson* parent)
{
    skJson  temp;
    skJson  array_node;
    skToken token;
    bool    err;       /* Error flag */
    bool    start;     /* First iteration flag */
    bool    parse_err; /* Parsed value error flag */

    start     = true;
    err       = false;
    parse_err = false;
    token     = skScanner_next(scanner);

    array_node = ArrayNode_new(parent);
    /* If arena allocation failed return immediately. */
    if(array_node.type == SK_NONE_NODE) {
        return array_node;
    }

    skScanner_skip(scanner, 2, SK_WS, SK_NL);

    /* If next token ']' return empty array '[]' */
    if(token.type == SK_RBRACK) {
        skScanner_next(scanner);
        return array_node;
    }

    do {
        if(!start) {
            skScanner_next(scanner);
            skScanner_skip(scanner, 2, SK_WS, SK_NL);
        } else {
            start = false;
        }

        temp = skJsonNode_parse(scanner, &array_node);

        if(temp.type == SK_NONE_NODE) {
            skJsonNode_drop(&array_node);
            set_none(array_node);
            return array_node;
        } else if(temp.type == SK_ERROR_NODE) {
            parse_err = err = true;
            break;
        }

        if(!skVec_push(array_node.data.j_array, &temp)) {
            skJsonNode_drop(&temp);
            skJsonNode_drop(&array_node);
            set_none(array_node);
            return array_node;
        }

        skScanner_skip(scanner, 2, SK_WS, SK_NL);

    } while((token = skScanner_peek(scanner)).type == SK_COMMA);

    /* Return ErrorNode if err occured or array isn't enclosed by ']' */
    if(err || token.type != SK_RBRACK) {
        skJsonNode_drop(&array_node);
        if(parse_err) {
            return temp;
        } else {
            return ErrorNode_new("failed parsing Json Array", scanner->iter.state, parent);
        }
    }

    skScanner_next(scanner);
    skScanner_skip(scanner, 2, SK_WS, SK_NL);
#ifdef SK_DBUG
    assert(is_some(array_node.data.j_array));
    assert(skVec_len(array_node.data.j_array) > 0);
#endif
    return array_node;
}

skJsonString skJsonString_new_internal(skScanner* scanner, skJson* err)
{
    size_t       bytes;
    skJsonString jstring;
    skStrSlice   slice;

    jstring = NULL;
    slice   = scanner->token.lexeme;

    if(slice.len > 0 && !skJsonString_isvalid(&slice)) {
        *err = ErrorNode_new("Invalid Json String\n", scanner->iter.state, NULL);
        return NULL;
    }

    bytes = slice.len + 1;
    if(is_null((jstring = malloc(bytes)))) {
#ifdef SK_ERRMSG
        THROW_ERR(OutOfMemory);
#endif
        return NULL;
    }

    jstring            = strncpy(jstring, slice.ptr, slice.len);
    jstring[slice.len] = '\0';
    return jstring;
}

bool skJsonString_isvalid(const skStrSlice* slice)
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

skJson skparse_json_string(skScanner* scanner, skJson* parent)
{
    skJsonString jstring;
    skJson       string_node;
    skJson       err_node;

    set_none(string_node);
    set_none(err_node);
    jstring = skJsonString_new_internal(scanner, &err_node);

    if(err_node.type == SK_ERROR_NODE) {
        return err_node;
    } else if(is_null(jstring)) {
        return string_node;
    }

    string_node               = RawNode_new(SK_STRING_NODE, parent);
    string_node.data.j_string = jstring;

    skScanner_next(scanner);
    return string_node;
}

skJson skparse_json_number(skScanner* scanner, skJson* parent)
{
    skJsonInteger integ;
    skJsonDouble  dbl;
    skToken       token;
    char*         start;
    char*         end;
    bool          negative;
    bool          integer;
    bool          fraction;

    negative = false;
    integer  = false;
    fraction = false;

    token = skScanner_peek(scanner);
    /* Start of the number stream */
    start = token.lexeme.ptr;

    if(token.type == SK_HYPHEN) {
        negative = true;
        token    = skScanner_next(scanner);
    }

    if(token.type == SK_DIGIT || token.type == SK_ZERO) {
        integer = true;

        if(token.type == SK_ZERO) {
            token = skScanner_next(scanner);
            if(token.type == SK_DIGIT || token.type == SK_ZERO) {
                goto jmp_err;
            }
        } else {
            skScanner_skip(scanner, 2, SK_DIGIT, SK_ZERO);
        }
    } else if(negative) {
        goto jmp_err;
    }

    if(skScanner_peek(scanner).type == SK_DOT) {
        if(!integer) {
            goto jmp_err;
        } else {
            skScanner_next(scanner);
        }

        if((token = skScanner_peek(scanner)).type == SK_DIGIT || token.type == SK_ZERO) {
            fraction = true;
            skScanner_skip(scanner, 2, SK_DIGIT, SK_ZERO);
        }

        if(!fraction) {
            goto jmp_err;
        }
    }

    if(skScanner_peek(scanner).type == SK_EXP) {
        if(!fraction) {
            goto jmp_err;
        }

        if((token = skScanner_next(scanner)).type != SK_HYPHEN && token.type != SK_PLUS) {
            goto jmp_err;
        }

        if((token = skScanner_next(scanner)).type == SK_DIGIT || token.type == SK_ZERO) {
            skScanner_skip(scanner, 2, SK_DIGIT, SK_ZERO);
        } else {
            goto jmp_err;
        }
    }

    /* End of number stream. */
    end = skScanner_peek(scanner).lexeme.ptr;

    errno = 0;
    if(fraction) {
        dbl = strtod(start, &end);
        if(errno == ERANGE) {
            THROW_WARN(OverflowDetected, scanner);
            dbl = DBL_MAX;
        }
        return DoubleNode_new(dbl, parent);
    } else {
        integ = strtol(start, &end, 10);
        if(errno == ERANGE) {
            THROW_WARN(OverflowDetected, scanner);
            integ = INT_MAX;
        }
        return IntNode_new(integ, parent);
    }

jmp_err:
    return ErrorNode_new("failed to parse Json Number", scanner->iter.state, parent);
}

skJson skparse_json_bool(skScanner* scanner, skJson* parent)
{
    skToken token;
    token = skScanner_peek(scanner);
    skScanner_next(scanner);
    return BoolNode_new(((token.type == SK_TRUE) ? true : false), parent);
}

skJson skparse_json_null(skScanner* scanner, skJson* parent)
{
    skScanner_next(scanner);
    return RawNode_new(SK_NULL_NODE, parent);
}
