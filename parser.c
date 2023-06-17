#include "node.h"
#include "parser.h"
#include "scanner.h"
#include "token.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

static Sk_JsonString Sk_JsonString_new(Sk_Token token);
static Sk_JsonNode*  Sk_parse_json_object(Sk_Scanner* scanner);
static Sk_JsonNode*  Sk_parse_json_array(Sk_Scanner* scanner);
static Sk_JsonNode*  Sk_parse_json_string(Sk_Scanner* scanner);
static Sk_JsonNode*  Sk_parse_json_number(Sk_Scanner* scanner);
static Sk_JsonNode*  Sk_parse_json_bool(Sk_Scanner* scanner);
static Sk_JsonNode*  Sk_parse_json_null(Sk_Scanner* scanner);

Sk_JsonNode*
Sk_JsonNode_new(Sk_Scanner* scanner)
{
    Sk_Token next = Sk_Scanner_peek(scanner);
    switch(next.type) {
        case SK_LCURLY:
            return Sk_parse_json_object(scanner);
        case SK_LBRACK:
            return Sk_parse_json_array(scanner);
        case SK_STRING:
            return Sk_parse_json_string(scanner);
        case SK_NUMBER:
            return Sk_parse_json_number(scanner);
        case SK_BOOL:
            return Sk_parse_json_bool(scanner);
        case SK_NULL:
            return Sk_parse_json_null(scanner);
        default:
            return Sk_JsonErrorNode_new("Invalid syntax while creating node");
    }
}

static Sk_JsonNode*
Sk_parse_json_object(Sk_Scanner* scanner)
{
    /// Error indicator
    bool err = false;
    /// Don't skip first token
    bool start = true;

    /// Current member we are parsing
    Sk_JsonMember temp;

    /// Collection of members for this object
    Sk_Vec members = Sk_Vec_new(sizeof(Sk_JsonMember));

    /// Take the already known token '{'
    Sk_Token token = Sk_Scanner_next(scanner);

    /// If next token is '}', return empty object '{}'
    if((token = Sk_Scanner_next(scanner)).type == SK_RCURLY) {
        return Sk_JsonObjectNode_new(members);
    }

    /// Else parse the member/s
    do {

        if(!start) {
            token = Sk_Scanner_next(scanner);
        } else {
            start = false;
        }

        if(token.type == SK_STRING) {
            temp.string = Sk_JsonString_new(token);

            if(temp.string == NULL) {
                err = true;
                break;
            }

            /// Fetch next token
            token = Sk_Scanner_next(scanner);

            if(token.type != SK_SEMICOLON) {
                err = true;
                break;
            }

            /// Fetch next token
            token = Sk_Scanner_next(scanner);

            /// Fetch value (enter recursion)
            temp.value = Sk_JsonNode_new(scanner);

            if(temp.value->type == SK_ERROR_NODE) {
                err = true;
                break;
            }
        }
        /// If token is not '}' or 'String', then error
        else
        {
            err = true;
            break;
        }

        /// Add new member to the object
        Sk_Vec_push(&members, &temp);

    } while((token = Sk_Scanner_next(scanner)).type == SK_COMMA);

    /// Return ErrorNode if err occured or object isn't enclosed by '}'.
    if(err || token.type != SK_RCURLY) {

        /// Cleanup the Members of the Object before returning error
        Sk_Vec_drop(&members, (FreeFn) Sk_JsonMember_drop);

        /// Return error
        return Sk_JsonErrorNode_new("failed parsing json object");
    }

    /// Return valid Json Object.
    return Sk_JsonObjectNode_new(members);
}

static Sk_JsonNode*
Sk_parse_json_array(Sk_Scanner* scanner)
{
    /// Error indicator
    bool err = false;

    /// Don't skip first token
    bool start = true;

    /// Current node we are parsing
    Sk_JsonNode* temp;

    /// Initialize storage for nodes
    Sk_Vec nodes = Sk_Vec_new(sizeof(Sk_JsonNode*));

    /// Fetch the already known token '['
    Sk_Token token = Sk_Scanner_next(scanner);

    /// If next token is ']' return empty array '[]'
    if((token = Sk_Scanner_next(scanner)).type == SK_RBRACK) {
        return Sk_JsonArrayNode_new(nodes);
    }

    /// Else parse the node/s
    do {

        if(!start) {
            token = Sk_Scanner_next(scanner);
        } else {
            start = false;
        }

        /// Parse the node
        temp = Sk_JsonNode_new(scanner);

        /// If node parsing errored set err
        if(temp->type == SK_ERROR_NODE) {
            err = true;
            break;
        }

        /// Add newly parsed node to the nodes
        Sk_Vec_push(&nodes, temp);

    } while((token = Sk_Scanner_next(scanner)).type == SK_COMMA);

    /// Return ErrorNode if err occured or array isn't enclosed by ']'
    if(err || token.type != SK_RBRACK) {

        /// Cleanup the Nodes of the Array before returning error
        Sk_Vec_drop(&nodes, (FreeFn) Sk_JsonNode_drop);

        /// Retrun error
        Sk_JsonErrorNode_new("failed parsing json array");
    }

    /// Return valid Json Array
    return Sk_JsonArrayNode_new(nodes);
}

static Sk_JsonString
Sk_JsonString_new(Sk_Token token)
{
    /// Number of hexadecimal code points for UTF-16 encoding
    static const u_char UNIC_HEX_UTF16_CODES = 4;

    Sk_JsonString jstring;
    int           c;
    bool          hex = false;

    Sk_CharIter iterator = Sk_CharIter_from_slice(&token.lexeme);

    while((c = Sk_CharIter_next(&iterator)) != EOF) {
        /// Valid char's from 0020 - 0FFFF (UTF-16)
        /// But our iterator iterates over char* so whatever...
        if(c < 020) {
            return NULL;
        }

        for(int count = 0; hex && count < UNIC_HEX_UTF16_CODES; count++) {
            if(!isxdigit(c)) {
                return NULL;
            }
        }

        hex = false;

        if(c == '\\') {
            c = Sk_CharIter_next(&iterator);

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

    size_t        bytes = token.lexeme.len;
    Sk_JsonString str   = malloc(bytes);

    if(str != NULL) {
        PRINT_OOM_ERR;
        return NULL;
    }

    str = strncpy(str, token.lexeme.ptr, bytes);
    return str;
}

static Sk_JsonNode*
Sk_parse_json_string(Sk_Scanner* scanner)
{
    /// Fetch the already known token 'SK_STRING'
    Sk_Token      token = Sk_Scanner_next(scanner);
    Sk_JsonString jstring;

    if((jstring = Sk_JsonString_new(token)) == NULL) {
        return Sk_JsonErrorNode_new("failed to parse Json String");
    }

    return Sk_JsonStringNode_new(jstring);
}

/// A bit wierd control flow in order to avoid nesting too much,
/// this could all be nested like this:
/// is_digit -> is_fraction -> is_exponent (would result in some code
/// being nested 5 levels deep...)
static Sk_JsonNode*
Sk_parse_json_number(Sk_Scanner* scanner)
{
    /// Fetch already known token 'SK_NUMBER'
    Sk_Token token = Sk_Scanner_next(scanner);

    int  c;
    bool digit    = false;
    bool zero     = false;
    bool sign     = false;
    bool err      = false;
    bool fraction = false;
    bool exponent = false;

    Sk_CharIter iterator = Sk_CharIter_from_slice(&token.lexeme);

    c = Sk_CharIter_next(&iterator);

    if(c == '-') {
        c = Sk_CharIter_next(&iterator);
    }

    if(isdigit(c)) {
        digit = true;

        if(c == '0') {
            zero = true;
            if(isdigit(Sk_CharIter_peek_next(&iterator))) {
                err = true;
                goto end;
            }
        }

        while(isdigit(c = Sk_CharIter_next(&iterator))) {
            ;
        }
    }

    if(c == '.') {
        if(!digit) {
            err = true;
            goto end;
        }

        while(isdigit(c = Sk_CharIter_next(&iterator))) {
            fraction = true;
        }

        if(!fraction) {
            err = true;
            goto end;
        }
    }

    if(c == 'e' || c == 'E') {
        if(!fraction) {
            err = true;
            goto end;
        }

        if((c = Sk_CharIter_next(&iterator)) == '-' || c == '+') {
            sign = true;
        }

        if(!sign) {
            err = true;
            goto end;
        }

        while(isdigit(c = Sk_CharIter_next(&iterator))) {
            ;
        }
    }

end:
    if(err || c != EOF) {
        return Sk_JsonErrorNode_new("failed to parse Json Number");
    } else if(fraction) {
        return Sk_JsonDoubleNode_new(atof(token.lexeme.ptr));
    } else {
        return Sk_JsonIntegerNode_new(atoi(token.lexeme.ptr));
    }
}

Sk_JsonNode*
Sk_parse_json_bool(Sk_Scanner* scanner)
{
    char* ptr = scanner->next.lexeme.ptr;

    if(strcmp(ptr, "true") == 0) {
        return Sk_JsonBoolNode_new(true);
    } else if(strcmp(ptr, "false") == 0) {
        return Sk_JsonBoolNode_new(false);
    } else {
        return Sk_JsonErrorNode_new("failed parsing Json Bool");
    }
}

Sk_JsonNode*
Sk_parse_json_null(Sk_Scanner* scanner)
{
    if(strcmp(scanner->next.lexeme.ptr, "null")) {
        return Sk_JsonNullNode_new();
    } else {
        return Sk_JsonErrorNode_new("failed parsing Json Null");
    }
}
