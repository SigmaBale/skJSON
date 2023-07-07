#include "sktypes.h"
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

skJsonString skJsonString_new_internal(skScanner* scanner, skJson* err_node);
skJson       skparse_json_object(skScanner* scanner, skJson* parent, bool* oom);
skJson       skparse_json_array(skScanner* scanner, skJson* parent, bool* oom);
skJson       skparse_json_string(skScanner* scanner, skJson* parent, bool* oom);
skJson       skparse_json_number(skScanner* scanner, skJson* parent, bool* oom);
skJson       skparse_json_bool(skScanner* scanner, skJson* parent);
skJson       skparse_json_null(skScanner* scanner, skJson* parent);

skJson skJsonNode_parse(skScanner* scanner, skJson* parent, bool* oom)
{
    skToken next;

    /* Peek at current token without advancing */
    next = skScanner_peek(scanner);
    switch(next.type) {
        case SK_LCURLY:
            return skparse_json_object(scanner, parent, oom);
        case SK_LBRACK:
            return skparse_json_array(scanner, parent, oom);
        case SK_STRING:
            return skparse_json_string(scanner, parent, oom);
        case SK_HYPHEN:
        case SK_ZERO:
        case SK_DIGIT:
            return skparse_json_number(scanner, parent, oom);
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

skJson skparse_json_object(skScanner* scanner, skJson* parent, bool* oom)
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
    *oom      = false;

    err_node.data.j_string = NULL;
    object_node            = ObjectNode_new(parent);
    /* Return immediately if allocation failed. */
    if(is_null(object_node.data.j_object)) {
        printf("Out of memory!\n");
        *oom = true;
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

            if(is_some(err_node.data.j_err)) {
                err = true;
                break;
            } else if(is_null(key)) {
                *oom = true;
                skJsonNode_drop(&object_node);
                return object_node;
            } else {
                skScanner_next(scanner);
                skScanner_skip(scanner, 1, SK_WS);
            }

            if(skScanner_peek(scanner).type != SK_COLON) {
                free(key);
                err = true;
                break;
            } else {
                skScanner_next(scanner);
                skScanner_skip(scanner, 1, SK_WS);
            }
#ifdef SK_DBUG
            assert(*oom == false);
#endif
            value = skJsonNode_parse(scanner, &object_node, oom);

            if(*oom) {
                free(key);
                skJsonNode_drop(&object_node);
                return object_node;
            } else if(value.type == SK_ERROR_NODE) {
                free(key);
                parse_err = err = true;
                break;
            } else {
                tuple.key   = key;
                tuple.value = value;
#ifdef SK_DBUG
                assert(tuple.value.parent_arena.ptr == object_node.data.j_object);
                assert(tuple.value.parent_arena.type == SK_OBJECT_NODE);
#endif
            }
        } else {
            err = true;
            break;
        }

        if(!skVec_push(object_node.data.j_object, &tuple)) {
            *oom = true;
            skObjTuple_drop(&tuple);
            skJsonNode_drop(&object_node);
            return object_node;
        }

        skScanner_skip(scanner, 2, SK_WS, SK_NL);

    } while((token = skScanner_peek(scanner)).type == SK_COMMA);

    if(err || token.type != SK_RCURLY) {
        skJsonNode_drop(&object_node);
        if(parse_err) {
            return value;
        } else {
            err_node
                = ErrorNode_new("failed parsing Json Object", scanner->iter.state, parent);

            if(is_null(err_node.data.j_err)) {
                *oom = true;
            }

            return err_node;
        }
    } else {
        skScanner_next(scanner);
        skScanner_skip(scanner, 2, SK_WS, SK_NL);
    }
#ifdef SK_DBUG
    assert(is_some(object_node.data.j_object));
    assert(skVec_len(object_node.data.j_object) > 0);
#endif
    return object_node;
}

skJson skparse_json_array(skScanner* scanner, skJson* parent, bool* oom)
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
    *oom      = false;
    token     = skScanner_next(scanner);

    array_node = ArrayNode_new(parent);
    /* If arena allocation failed return immediately. */
    if(is_null(array_node.data.j_array)) {
        *oom = true;
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
#ifdef SK_DBUG
        assert(*oom == false);
#endif
        temp = skJsonNode_parse(scanner, &array_node, oom);

        if(*oom) {
            skJsonNode_drop(&array_node);
            return array_node;
        } else if(temp.type == SK_ERROR_NODE) {
            parse_err = err = true;
            break;
        }

        if(!skVec_push(array_node.data.j_array, &temp)) {
            *oom = true;
            skJsonNode_drop(&temp);
            skJsonNode_drop(&array_node);
            return array_node;
        } else {
            skScanner_skip(scanner, 2, SK_WS, SK_NL);
        }

    } while((token = skScanner_peek(scanner)).type == SK_COMMA);

    /* Return ErrorNode if err occured or array isn't enclosed by ']' */
    if(err || token.type != SK_RBRACK) {
        skJsonNode_drop(&array_node);
        if(parse_err) {
            return temp;
        } else {
            temp = ErrorNode_new("failed parsing Json Array", scanner->iter.state, parent);
            if(is_null(temp.data.j_err)) {
                *oom = true;
            }
            return temp;
        }
    } else {
        skScanner_next(scanner);
        skScanner_skip(scanner, 2, SK_WS, SK_NL);
    }
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

skJson skparse_json_string(skScanner* scanner, skJson* parent, bool* oom)
{
    skJsonString jstring;
    skJson       string_node;
    skJson       err_node;

    memset(&string_node, 0, sizeof(skJson));
    *oom                = false;
    err_node.data.j_err = NULL;
    jstring             = skJsonString_new_internal(scanner, &err_node);

    if(is_some(err_node.data.j_err)) {
        return err_node;
    } else if(is_null(jstring)) {
        *oom = true;
        return string_node;
    }

    string_node               = RawNode_new(SK_STRING_NODE, parent);
    string_node.data.j_string = jstring;

    skScanner_next(scanner);
#ifdef SK_DBUG
    assert(*oom == false);
#endif
    return string_node;
}

skJson skparse_json_number(skScanner* scanner, skJson* parent, bool* oom)
{
    skJsonInteger integ;
    skJsonDouble  dbl;
    skJson        err_node;
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
    err_node = ErrorNode_new("failed to parse Json Number", scanner->iter.state, parent);
    if(is_null(err_node.data.j_err)) {
        *oom = true;
    }
    return err_node;
}

skJson skparse_json_bool(skScanner* scanner, skJson* parent)
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

skJson skparse_json_null(skScanner* scanner, skJson* parent)
{
    skScanner_next(scanner);
    return RawNode_new(SK_NULL_NODE, parent);
}
