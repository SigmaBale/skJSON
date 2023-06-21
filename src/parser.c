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

Sk_JsonString Sk_JsonString_new(Sk_Token token);
Sk_JsonNode*  Sk_parse_json_object(Sk_Scanner* scanner);
Sk_JsonNode*  Sk_parse_json_array(Sk_Scanner* scanner);
Sk_JsonNode*  Sk_parse_json_string(Sk_Scanner* scanner);
Sk_JsonNode*  Sk_parse_json_number(Sk_Scanner* scanner);
Sk_JsonNode*  Sk_parse_json_bool(Sk_Scanner* scanner);
Sk_JsonNode*  Sk_parse_json_null();

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

    /// Construct the parse tree
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
        case SK_ZERO:
        case SK_DIGIT:
            return Sk_parse_json_number(scanner);
        case SK_FALSE:
        case SK_TRUE:
            return Sk_parse_json_bool(scanner);
        case SK_NULL:
            return Sk_parse_json_null();
        case SK_INVALID:
        default:
            return Sk_JsonErrorNode_new("Invalid syntax/token while parsing");
    }
}

Sk_JsonNode*
Sk_parse_json_object(Sk_Scanner* scanner)
{
    /// Error indicator
    bool err = false;

    /// Current member we are parsing
    Sk_JsonMember temp;

    /// Collection of members for this object
    Sk_Vec members = Sk_Vec_new(sizeof(Sk_JsonMember));

    /// Take the already known token '{'
    Sk_Token token = Sk_Scanner_next(scanner);

    /// Skip whitespace (and tabs) and newlines
    Sk_Scanner_skip(scanner, 2, SK_WS, SK_NL);

    /// If next token is '}', return empty object '{}'
    if((token = Sk_Scanner_next(scanner)).type == SK_RCURLY) {
        return Sk_JsonObjectNode_new(members);
    }

    /// Else parse the member/s
    do {

        Sk_Scanner_skip(scanner, 2, SK_WS, SK_NL);

        if(token.type == SK_STRING) {
            temp.string = Sk_JsonString_new(token);

            if(temp.string == NULL) {
                err = true;
                break;
            }

            /// Skip whitespaces
            Sk_Scanner_skip(scanner, 1, SK_WS);
            /// Fetch current non-whitespace token
            token = Sk_Scanner_peek(scanner);

            if(token.type != SK_COLON) {
                err = true;
                break;
            }

            /// Skip whitespaces
            Sk_Scanner_skip(scanner, 1, SK_WS);

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

        Sk_Scanner_skip(scanner, 2, SK_WS, SK_NL);

    } while((token = Sk_Scanner_peek(scanner)).type == SK_COMMA);

    /// Return ErrorNode if err occured or object isn't enclosed by '}'.
    if(err || token.type != SK_RCURLY) {

        /// Cleanup the Members of the Object before returning error
        Sk_Vec_drop(&members, (FreeFn) Sk_JsonMember_drop);

        /// Skip this depth where error occured so we can
        /// continue parsing the rest of the file
        Sk_CharIter_depth_above(&scanner->iter);

        /// Return error
        return Sk_JsonErrorNode_new("failed parsing json object");
    }

    /// Return valid Json Object.
    return Sk_JsonObjectNode_new(members);
}

Sk_JsonNode*
Sk_parse_json_array(Sk_Scanner* scanner)
{
    /// Error indicator
    bool err = false;

    /// Current node we are parsing
    Sk_JsonNode* temp;

    /// Initialize storage for nodes
    Sk_Vec nodes = Sk_Vec_new(sizeof(Sk_JsonNode*));

    /// Skip spaces
    Sk_Scanner_skip(scanner, 2, SK_WS, SK_NL);
    Sk_Token token = Sk_Scanner_peek(scanner);

    /// If next token ']' return empty array '[]'
    if(token.type == SK_RBRACK) {
        return Sk_JsonArrayNode_new(nodes);
    }

    /// Else parse the node/s
    do {
        /// Skip spaces (does nothing first iteration)
        Sk_Scanner_skip(scanner, 2, SK_WS, SK_NL);

        /// Parse the node
        temp = Sk_JsonNode_new(scanner);

        /// If node parsing errored set err
        if(temp->type == SK_ERROR_NODE) {
            err = true;
            break;
        }

        /// Add newly parsed node to the nodes
        Sk_Vec_push(&nodes, temp);

        Sk_Scanner_skip(scanner, 2, SK_WS, SK_NL);

    } while((token = Sk_Scanner_peek(scanner)).type == SK_COMMA);

    /// Return ErrorNode if err occured or array isn't enclosed by ']'
    if(err || token.type != SK_RBRACK) {

        /// Cleanup the Nodes of the Array before returning error
        Sk_Vec_drop(&nodes, (FreeFn) Sk_JsonNode_drop);

        /// Skip this depth where error occured, so we can
        /// continue parsing the rest of the file
        Sk_CharIter_depth_above(&scanner->iter);

        /// Retrun error
        return Sk_JsonErrorNode_new("failed parsing json array");
    }

    /// Return valid Json Array
    return Sk_JsonArrayNode_new(nodes);
}

/// Checks the validity of a string and allocates
/// storage for it if the json string is valid
Sk_JsonString
Sk_JsonString_new(Sk_Token token)
{
    /// Number of hexadecimal code points for UTF-16 encoding
    static const unsigned char UNIC_HEX_UTF16_CODES = 4;

    /// If string is empty "" return the empty string...
    if(token.lexeme.len == 0) {
        return SK_EMPTY_STRING;
    }

    int  c;
    bool hex = false;

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

    size_t        bytes   = token.lexeme.len;
    Sk_JsonString jstring = malloc(bytes + 1); /// +1 for null terminator

    /// Allocation failed
    if(jstring == NULL) {
        PRINT_OOM_ERR;
        return NULL;
    }

    /// Copy over 'bytes' amount into str
    jstring = strncpy(jstring, token.lexeme.ptr, bytes);
    /// Null terminate the str (that's why we added +1 to malloc)
    jstring[bytes] = '\0';
#ifdef SK_DBUG
    assert(strlen(jstring) == bytes);
#endif
    return jstring;
}

Sk_JsonNode*
Sk_parse_json_string(Sk_Scanner* scanner)
{
    /// Get the SK_STRING token
    Sk_Token      token   = Sk_Scanner_peek(scanner);
    Sk_JsonString jstring = NULL;

    /// Parse the Json String
    if((jstring = Sk_JsonString_new(token)) == NULL) {
        Sk_CharIter_depth_above(&scanner->iter);
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
Sk_JsonNode*
Sk_parse_json_number(Sk_Scanner* scanner)
{
    /// Fetch the first token (either a digit, zero or hyphen)
    Sk_Token token = Sk_Scanner_peek(scanner);
    /// Store the start of the Json Number
    char*    start = Sk_CharIter_next_address(&scanner->iter) - 1;

    /// Flags for control flow and validity check
    bool negative = false;
    bool integer  = false;
    bool fraction = false;

    /// If first token is HYPHEN
    if(token.type == SK_HYPHEN) {
        /// We have a 'negative' number
        negative = true;
        token    = Sk_Scanner_next(scanner);
    }

    /// If we have a digit (1-9) or zero (0) parse the integer part
    if(token.type == SK_DIGIT || token.type == SK_ZERO) {
        /// We have integer
        integer = true;

        /// Integer part is zero
        if(token.type == SK_ZERO) {
            token = Sk_Scanner_next(scanner);
            if(token.type == SK_DIGIT || token.type == SK_ZERO) {
                goto jmp_err;
            }
        } else {
            /// If we got digit (1-9), skip them all including
            /// zeroes until we get non digit and non zero token
            Sk_Scanner_skip(scanner, 2, SK_DIGIT, SK_ZERO);
        }
    } else if(negative) {
        /// If we didn't find integer part but we have a HYPHEN
        /// aka minus sign, that is invalid number, goto error
        goto jmp_err;
    }

    /// Let's check if there is fraction
    token = Sk_Scanner_peek(scanner);

    if(token.type == SK_DOT) {
        /// If we have a decimal point but no
        /// integer part, that is invalid json number
        if(!integer) {
            goto jmp_err;
        } else {
            Sk_Scanner_next(scanner);
        }

        /// If we have a digit/zero after decimal point then we have a valid fraction
        if((token = Sk_Scanner_peek(scanner)).type == SK_DIGIT || token.type == SK_ZERO) {
            /// Set fraction as valid
            fraction = true;
            /// Skip the rest of digits
            Sk_Scanner_skip(scanner, 2, SK_DIGIT, SK_ZERO);
        }

        /// If fraction is invalid, goto error
        if(!fraction) {
            goto jmp_err;
        }
    }

    /// Let's check if there is exponent
    token = Sk_Scanner_peek(scanner);

    if(token.type == SK_EXP) {
        /// If there is no fractional part, then we can't have
        /// exponent part, goto error
        if(!fraction) {
            goto jmp_err;
        }

        /// Check if exponent has a sign
        if((token = Sk_Scanner_next(scanner)).type != SK_HYPHEN && token.type != SK_PLUS) {
            /// If exponent doesn't have a sign it is invalid, goto error
            goto jmp_err;
        }

        /// Exponent must contain digits/zero-es in order to be valid
        if((token = Sk_Scanner_next(scanner)).type == SK_DIGIT || token.type == SK_ZERO) {
            /// Exponent is valid, skip the rest of the digits/zeroes
            Sk_Scanner_skip(scanner, 2, SK_DIGIT, SK_ZERO);
        } else {
            /// Exponent is invalid, it contains no digits, goto error
            goto jmp_err;
        }
    }

    /// Set the end of the token stream
    char*         end = Sk_CharIter_next_address(&scanner->iter);
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
    /// Skip entire json object/array to continue parsing rest of the file
    Sk_CharIter_depth_above(&scanner->iter);
    /// Finally return the error node
    return Sk_JsonErrorNode_new("failed to parse Json Number");
}

void
print_node(Sk_JsonNode* node)
{
    switch(node->type) {
        case SK_ERROR_NODE:
            printf("ERROR NODE\n");
            break;
        case SK_OBJECT_NODE:
            printf("OBJECT NODE\n");
            break;
        case SK_ARRAY_NODE:
            printf("ARRAY NODE\n");
            break;
        case SK_STRING_NODE:
            printf("STRING NODE\n");
            break;
        case SK_EMPTYSTRING_NODE:
            printf("EMPTYSTRING NODE\n");
            break;
        case SK_INT_NODE:
            printf("INT NODE\n");
            break;
        case SK_DOUBLE_NODE:
            printf("DOUBLE NODE\n");
            break;
        case SK_BOOL_NODE:
            printf("BOOL NODE\n");
            break;
        case SK_NULL_NODE:
            printf("NULL NODE\n");
            break;
    }
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
Sk_parse_json_null()
{
    /// Just construct Json Null node
    return Sk_JsonNullNode_new();
}
