#include "parser.h"
#include "token.h"

static Sk_JsonNode* Sk_parse_json_object(Sk_Scanner* scanner);
static Sk_JsonNode* Sk_parse_json_array(Sk_Scanner* scanner);
static Sk_JsonNode* Sk_parse_json_string(Sk_Scanner* scanner);
static Sk_JsonNode* Sk_parse_json_number(Sk_Scanner* scanner);
static Sk_JsonNode* Sk_parse_json_true(Sk_Scanner* scanner);
static Sk_JsonNode* Sk_parse_json_false(Sk_Scanner* scanner);
static Sk_JsonNode* Sk_parse_json_null(Sk_Scanner* scanner);

Sk_JsonNode*
json_node_new(Sk_Scanner* scanner)
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
    if(scanner == NULL) {
        return NULL;
    }

    bool rcurly    = false;
    bool member    = false;
    bool string    = false;
    bool semicolon = false;
    bool comma     = false;
    bool invalid   = false;

    Sk_JsonMember   temp    = (Sk_JsonMember) { .string = NULL, .value = 0 };
    Sk_JsonMember** members = NULL;
    size_t          len     = 0;
    Sk_Token        token;
    while(!rcurly && !invalid && (token = Sk_Scanner_next(scanner)).type != SK_END_TOK) {
        switch(token.type) {
            case SK_RCURLY_TOK:
                if(member && !comma) {
                    member = false;
                    rcurly = true;
                } else {
                    invalid = true;
                }
                break;
            case SK_COMMA_TOK:
                if(!member) {
                    invalid = true;
                } else {
                    comma  = true;
                    member = false;
                }
                break;
            case SK_STRING_TOK:
                if(!semicolon) {
                    string = true;
                    
                }

            default:
                invalid = true;
                break;
        }
    }

    if(token.type == SK_END_TOK) {
        return NULL;
    }

    return NULL;
}
