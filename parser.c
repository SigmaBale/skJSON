#include "node.h"
#include "parser.h"
#include "scanner.h"
#include "token.h"
#include <stdlib.h>
#include <string.h>

static Sk_JsonString Sk_JsonString_new(Sk_Token token);
static Sk_JsonNode*  Sk_parse_json_object(Sk_Scanner* scanner);
static Sk_JsonNode*  Sk_parse_json_array(Sk_Scanner* scanner);
static Sk_JsonNode*  Sk_parse_json_string(Sk_Scanner* scanner);
static Sk_JsonNode*  Sk_parse_json_number(Sk_Scanner* scanner);
static Sk_JsonNode*  Sk_parse_json_true(Sk_Scanner* scanner);
static Sk_JsonNode*  Sk_parse_json_false(Sk_Scanner* scanner);
static Sk_JsonNode*  Sk_parse_json_null(Sk_Scanner* scanner);

Sk_JsonNode*
Sk_JsonNode_new(Sk_Scanner* scanner)
{
    Sk_Token next = Sk_Scanner_peek(scanner);
    switch(next.type) {
        case SK_LCURLY_TOK:
            return Sk_parse_json_object(scanner);
        case SK_LBRACK_TOK:
            return Sk_parse_json_array(scanner);
        case SK_STRING_TOK:
            return Sk_parse_json_string(scanner);
        case SK_NUMBER_TOK:
            return Sk_parse_json_number(scanner);
        case SK_TRUE_TOK:
            return Sk_parse_json_false(scanner);
        case SK_FALSE_TOK:
            return Sk_parse_json_true(scanner);
        case SK_NULL_TOK:
            return Sk_parse_json_null(scanner);
        default:
            return Sk_JsonErrorNode_new("I need to make better err handling...");
    }
}

static Sk_JsonNode*
Sk_parse_json_object(Sk_Scanner* scanner)
{
    /// TODO: Ensure we are passing non null scanner
    if(scanner == NULL) {
        return NULL;
    }

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
    if((token = Sk_Scanner_next(scanner)).type == SK_RCURLY_TOK) {
        return Sk_JsonObjectNode_new(members);
    }

    /// Else parse the member/s
    do {

        if(!start) {
            token = Sk_Scanner_next(scanner);
        } else {
            start = false;
        }

        if(token.type == SK_STRING_TOK) {
            temp.string = Sk_JsonString_new(token);

            if(temp.string == NULL) {
                err = true;
                break;
            }

            /// Fetch next token
            token = Sk_Scanner_next(scanner);

            if(token.type != SK_SEMICOLON_TOK) {
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

    } while((token = Sk_Scanner_next(scanner)).type == SK_COMMA_TOK);

    /// Return ErrorNode if err occured or object isn't enclosed by '}'.
    if(err || token.type != SK_RCURLY_TOK) {

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
    if(scanner == NULL) {
        return NULL;
    }

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
    if((token = Sk_Scanner_next(scanner)).type == SK_RBRACK_TOK) {
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

    } while((token = Sk_Scanner_next(scanner)).type == SK_COMMA_TOK);

    /// Return ErrorNode if err occured or array isn't enclosed by ']'
    if(err || token.type != SK_RBRACK_TOK) {

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
    if(scanner == NULL) {
        return NULL;
    }

    /// Fetch the already known token 'SK_STRING_TOK'
    Sk_Token token = Sk_Scanner_next(scanner);

    Sk_JsonString jstring = Sk_JsonString_new(token);

    return (jstring == NULL) ? NULL : Sk_JsonStringNode_new(jstring);
}

static Sk_JsonNode*
Sk_parse_json_number(Sk_Scanner* scanner)
{
    if(scanner == NULL) {
        return NULL;
    }

    /// Fetch already known token 'SK_NUMBER_TK'
    Sk_Token token = Sk_Scanner_next(scanner);

    switch((token = Sk_Scanner_next(scanner)).type) {
    }

    return NULL;
}
