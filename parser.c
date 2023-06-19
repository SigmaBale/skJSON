// clang-format off
#include "parser.h"
#include "token.h"
#include "node.h"
#include "scanner.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
// clang-format on

#define SK_EMPTY_STRING ""

static Sk_JsonString Sk_JsonString_new(Sk_Token token);
static Sk_JsonNode*  Sk_parse_json_object(Sk_Scanner* scanner);
static Sk_JsonNode*  Sk_parse_json_array(Sk_Scanner* scanner);
static Sk_JsonNode*  Sk_parse_json_string(Sk_Scanner* scanner);
static Sk_JsonNode*  Sk_parse_json_number(Sk_Scanner* scanner);
static Sk_JsonNode*  Sk_parse_json_bool(Sk_Scanner* scanner);
static Sk_JsonNode*  Sk_parse_json_null(Sk_Scanner* scanner);

struct skJson {
    Sk_JsonNode* root;
};

skJson*
sk_json_new(void* buff, size_t bufsize)
{
    if(buff == NULL) {
        return NULL;
    }

    Sk_JsonNode* root;
    Sk_Scanner*  scanner = Sk_Scanner_new(buff, bufsize);

    root = Sk_JsonNode_new(scanner);

    if(root == NULL) {
        return NULL;
    }

    skJson* json = malloc(sizeof(skJson));

    if(json == NULL) {
        PRINT_OOM_ERR;
        return NULL;
    }

    json->root = root;

    return json;
}

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
        case SK_HYPHEN:
        case SK_DIGIT:
            return Sk_parse_json_number(scanner);
        case SK_FALSE:
        case SK_TRUE:
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

            if(token.type != SK_COLON) {
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
        return Sk_JsonErrorNode_new("failed parsing json array");
    }

    /// Return valid Json Array
    return Sk_JsonArrayNode_new(nodes);
}

static Sk_JsonString
Sk_JsonString_new(Sk_Token token)
{
    if(token.lexeme.len == 0) {
        return SK_EMPTY_STRING;
    }

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
    Sk_JsonString str   = malloc(bytes + 1); /// +1 for '\0'

    if(str != NULL) {
        PRINT_OOM_ERR;
        return NULL;
    }

    str        = strncpy(str, token.lexeme.ptr, bytes);
    str[bytes] = '\0';
    assert(strlen(str) == bytes + 1);
    return str;
}

static Sk_JsonNode*
Sk_parse_json_string(Sk_Scanner* scanner)
{
    /// Fetch the already known token 'SK_STRING'
    Sk_Token      token   = Sk_Scanner_next(scanner);
    Sk_JsonString jstring = NULL;

    if((jstring = Sk_JsonString_new(token)) == NULL) {
        return Sk_JsonErrorNode_new("failed to parse Json String");
    }

    return Sk_JsonStringNode_new(jstring);
}

/// A bit wierd control flow in order to avoid nesting too much, this could
/// all be nested like this:
/// is_digit -> is_fraction -> is_exponent
/// (would result in some code being nested 5 levels deep...)
static Sk_JsonNode*
Sk_parse_json_number(Sk_Scanner* scanner)
{
    /// Fetch the first token
    Sk_Token token = Sk_Scanner_next(scanner);

    int  c;
    bool zero          = false;
    bool digit         = false;
    bool decimal_point = false;
    bool fraction      = false;
    bool exp           = false;
    bool sign          = false;
    bool err           = false;

    char* start = token.lexeme.ptr;

    if(token.type == SK_HYPHEN) {
        token = Sk_Scanner_next(scanner);
    }

    if(token.type == SK_DIGIT || token.type == SK_ZERO) {
        digit = true;

        if(token.type == SK_ZERO) {
            zero = true;
            if(Sk_Scanner_peek(scanner).type == SK_DIGIT) {
                err = true;
                goto jmp_err;
            }
        } else {
            Sk_Scanner_skip(scanner, SK_DIGIT);
        }
    }

    /// Let's check if there is fraction
    token = Sk_Scanner_next(scanner);

    if(token.type == SK_DOT) {
        if(!digit) {
            err = true;
            goto jmp_err;
        }

        if(Sk_Scanner_peek(scanner).type == SK_DIGIT) {
            fraction = true;
            Sk_Scanner_skip(scanner, SK_DIGIT);
        }

        if(!fraction) {
            err = true;
            goto jmp_err;
        }
    }

    /// Let's check if there is exponent
    token = Sk_Scanner_next(scanner);

    if(token.type == SK_EXP) {
        if(!fraction) {
            err = true;
            goto jmp_err;
        }

        Sk_Token peeked;

        if((peeked = Sk_Scanner_peek(scanner)).type == SK_HYPHEN || peeked.type == SK_PLUS) {
            sign  = true;
            token = Sk_Scanner_next(scanner);
        }

        if(!sign) {
            err = true;
            goto jmp_err;
        }

        if(Sk_Scanner_peek(scanner).type == SK_DIGIT) {
            token = Sk_Scanner_next(scanner);
            Sk_Scanner_skip(scanner, SK_DIGIT);
        } else {
            err = true;
            goto jmp_err;
        }
    }

    char*         end    = token.lexeme.ptr;
    Sk_JsonDouble number = strtod(start, &end);

    if(errno == ERANGE) {
        fprintf(stderr, "warning: Json Number overflow detected\n");
    }

    if(c != EOF) {
jmp_err:
        return Sk_JsonErrorNode_new("failed to parse Json Number");
    }

    if(fraction) {
        return Sk_JsonDoubleNode_new(number);
    } else {
        return Sk_JsonIntegerNode_new((Sk_JsonInteger) number);
    }
}

Sk_JsonNode*
Sk_parse_json_bool(Sk_Scanner* scanner)
{
    /// Fetch the bool token
    Sk_Token token = Sk_Scanner_next(scanner);

    if(token.type == SK_TRUE) {
        return Sk_JsonBoolNode_new(true);
    } else {
        return Sk_JsonBoolNode_new(false);
    }
}

inline Sk_JsonNode*
Sk_parse_json_null(Sk_Scanner* scanner)
{
    /// Fetch the null token
    Sk_Scanner_next(scanner);
    return Sk_JsonNullNode_new();
}
