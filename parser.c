// clang-format off
#include "token.h"
#include "scanner.h"
#include "node.h"
#include "parser.h"
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

    if(scanner == NULL) {
        return NULL;
    }

    skJson* json = malloc(sizeof(skJson));

    if(json == NULL) {
        free(scanner);
        PRINT_OOM_ERR;
        return NULL;
    }

    /// Fetch first token and construct the root node
    Sk_Scanner_next(scanner);
    root = Sk_JsonNode_new(scanner);

    if(root == NULL) {
        free(scanner);
        free(json);
        return NULL;
    }

    json->root = root;

    return json;
}

Sk_JsonNode*
Sk_JsonNode_new(Sk_Scanner* scanner)
{
    /// Peek at current token without advancing
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
        case SK_INVALID:
        default:
            return Sk_JsonErrorNode_new("Invalid syntax/token while parsing");
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

    /// Skip whitespace (and tabs) and newlines
    Sk_Scanner_skip(scanner, SK_WS, SK_NL);

    /// If next token is '}', return empty object '{}'
    if((token = Sk_Scanner_next(scanner)).type == SK_RCURLY) {
        return Sk_JsonObjectNode_new(members);
    }

    /// Else parse the member/s
    do {

        if(!start) {
            Sk_Scanner_skip(scanner, SK_WS, SK_NL);
            token = Sk_Scanner_peek(scanner);
        } else {
            start = false;
        }

        if(token.type == SK_STRING) {
            temp.string = Sk_JsonString_new(token);

            if(temp.string == NULL) {
                err = true;
                break;
            }

            /// Skip whitespaces
            Sk_Scanner_skip(scanner, SK_WS);
            /// Fetch current non-whitespace token
            token = Sk_Scanner_peek(scanner);

            if(token.type != SK_COLON) {
                err = true;
                break;
            }

            /// Skip whitespaces
            Sk_Scanner_skip(scanner, SK_WS);

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

        Sk_Scanner_skip(scanner, SK_WS, SK_NL);

    } while((token = Sk_Scanner_peek(scanner)).type == SK_COMMA);

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

    /// Skip spaces
    Sk_Scanner_skip(scanner, SK_WS, SK_NL);
    Sk_Token token = Sk_Scanner_peek(scanner);

    /// If next token ']' return empty array '[]'
    if(token.type == SK_RBRACK) {
        return Sk_JsonArrayNode_new(nodes);
    }

    /// Else parse the node/s
    do {
        /// Skip spaces (does nothing first iteration)
        Sk_Scanner_skip(scanner, SK_WS, SK_NL);

        /// Parse the node
        temp = Sk_JsonNode_new(scanner);

        /// If node parsing errored set err
        if(temp->type == SK_ERROR_NODE) {
            err = true;
            break;
        }

        /// Add newly parsed node to the nodes
        Sk_Vec_push(&nodes, temp);

        Sk_Scanner_skip(scanner, SK_WS, SK_NL);

    } while((token = Sk_Scanner_peek(scanner)).type == SK_COMMA);

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

/// Checks the validity of a string and allocates
/// storage for it if the json string is valid
static Sk_JsonString
Sk_JsonString_new(Sk_Token token)
{
    /// Number of hexadecimal code points for UTF-16 encoding
    static const u_char UNIC_HEX_UTF16_CODES = 4;

    /// If string is empty "" return the empty string...
    if(token.lexeme.len == 0) {
        return SK_EMPTY_STRING;
    }

    Sk_JsonString jstring;
    int           c;
    bool          hex = false;

    Sk_CharIter iterator = Sk_CharIter_from_slice(&token.lexeme);

    while((c = Sk_CharIter_next(&iterator)) != EOF) {
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
    Sk_JsonString str   = malloc(bytes + 1); /// +1 for null terminator

    /// Allocation failed
    if(str == NULL) {
        PRINT_OOM_ERR;
        return NULL;
    }

    /// Copy over 'bytes' amount into str
    str = strncpy(str, token.lexeme.ptr, bytes);
    /// Null terminate the str (that's why we added +1 to malloc)
    str[bytes] = '\0';
    /// Sanity check
    assert(strlen(str) == bytes + 1);
    return str;
}

static Sk_JsonNode*
Sk_parse_json_string(Sk_Scanner* scanner)
{
    /// Get the SK_STRING token
    Sk_Token      token   = Sk_Scanner_peek(scanner);
    Sk_JsonString jstring = NULL;

    /// Parse the Json String
    if((jstring = Sk_JsonString_new(token)) == NULL) {
        /// Parsing failed, return error
        return Sk_JsonErrorNode_new("failed to parse Json String");
    }

    /// Return valid Json String (null terminated)
    return Sk_JsonStringNode_new(jstring);
}

/// A bit wierd control flow in order to avoid nesting too much, this could
/// all be nested like this:
/// is_digit -> is_fraction -> is_exponent
/// (would result in some code being nested 5 levels deep...)
static Sk_JsonNode*
Sk_parse_json_number(Sk_Scanner* scanner)
{
    /// Fetch the first DIGIT or HYPHEN token
    Sk_Token token = Sk_Scanner_peek(scanner);

    /// Mark the start of the Json Number
    char* start = Sk_CharIter_current(&scanner->iter);

    /// Flags for control flow and validity check
    bool negative      = false;
    bool zero          = false;
    bool integer       = false;
    bool decimal_point = false;
    bool fraction      = false;
    bool exp           = false;
    bool sign          = false;
    bool err           = false;

    /// If first token is HYPHEN
    if(token.type == SK_HYPHEN) {
        /// We have a 'negative' number
        negative = true;
        /// Then just skip and fetch new one
        token = Sk_Scanner_next(scanner);
    }

    /// If we have a digit (1-9) or zero (0) parse the integer part
    if(token.type == SK_DIGIT || token.type == SK_ZERO) {
        /// We have integer
        integer = true;

        if(token.type == SK_ZERO) {
            /// Integer is zero
            zero = true;
            /// Fetch the next token
            token = Sk_Scanner_next(scanner);
            /// If token is a digit that is invalid
            /// integer, goto error
            if(token.type == SK_DIGIT) {
                err = true;
                goto jmp_err;
            }
        } else {
            /// If we got digit (1-9), skip them all including
            /// zeroes until we git non digit and non zero token
            Sk_Scanner_skip(scanner, SK_DIGIT, SK_ZERO);
        }
    }
    /// If we didn't find integer part but we have a HYPHEN
    /// aka minus sign, that is invalid number, goto error
    else if(negative)
    {
        err = true;
        goto jmp_err;
    }

    /// Let's check if there is fraction
    token = Sk_Scanner_peek(scanner);

    if(token.type == SK_DOT) {
        /// If we have a decimal point but no
        /// integer part, that is invalid json number
        if(!integer) {
            err = true;
            goto jmp_err;
        }

        /// If we have a digit/zero after decimal point then we have a valid fraction
        if((token = Sk_Scanner_peek(scanner)).type == SK_DIGIT || token.type == SK_ZERO) {
            /// Set fraction as valid
            fraction = true;
            /// Skip the rest of digits
            Sk_Scanner_skip(scanner, SK_DIGIT, SK_ZERO);
        }

        /// If fraction is invalid, goto error
        if(!fraction) {
            err = true;
            goto jmp_err;
        }
    }

    /// Let's check if there is exponent
    token = Sk_Scanner_peek(scanner);

    if(token.type == SK_EXP) {
        /// If there is no fractional part, then we can't have
        /// exponent part, goto error
        if(!fraction) {
            err = true;
            goto jmp_err;
        }

        /// Check if exponent has a sign
        if((token = Sk_Scanner_next(scanner)).type == SK_HYPHEN || token.type == SK_PLUS) {
            /// Exponent has a sign, keep parsing
            sign = true;
            /// Fetch the next token
            token = Sk_Scanner_next(scanner);
        }

        /// If exponent doesn't have a sign it is invalid, goto error
        if(!sign) {
            err = true;
            goto jmp_err;
        }

        /// Exponent must contain digits/zero-es in order to be valid
        if(Sk_Scanner_peek(scanner).type == SK_DIGIT) {
            /// Exponent is valid, skip the rest of the digits/zeroes
            Sk_Scanner_skip(scanner, SK_DIGIT, SK_ZERO);
        }
        /// Exponent is invalid, it contains no digits, goto error
        else
        {
            err = true;
            goto jmp_err;
        }
    }

    /// Set the end of the token stream
    char*         end = scanner->token.lexeme.ptr;
    /// Convert the stream into number (double)
    Sk_JsonDouble number = strtod(start, &end);

    /// If during conversion overflow occured, warn the user
    /// TODO: More direct error propagation to user
    if(errno == ERANGE) {
        fprintf(stderr, "warning: Json Number overflow detected\n");
    }

    /// If we have a fractional part, return double
    if(fraction) {
        return Sk_JsonDoubleNode_new(number);
    }
    /// Otherwise return integer (cast it back to int)
    else
    {
        return Sk_JsonIntegerNode_new((Sk_JsonInteger) number);
    }

jmp_err:
    /// We errored during parsing
    return Sk_JsonErrorNode_new("failed to parse Json Number");
}

Sk_JsonNode*
Sk_parse_json_bool(Sk_Scanner* scanner)
{
    /// Fetch the bool token
    Sk_Token token = Sk_Scanner_peek(scanner);

    if(token.type == SK_TRUE) {
        /// token is SK_TRUE, create True BoolNode
        return Sk_JsonBoolNode_new(true);
    } else {
        /// token is SK_FALSE, create False BoolNode
        return Sk_JsonBoolNode_new(false);
    }
}

inline Sk_JsonNode*
Sk_parse_json_null(Sk_Scanner* scanner)
{
    /// Just construct Json Null node
    return Sk_JsonNullNode_new();
}
