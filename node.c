#include "node.h"
#include <stdbool.h>
#include <stdint.h>


JsonNode*
json_node_new(Scanner* scanner)
{
    Token next = scanner_peek(scanner);
    switch(next.type) {
        case LCURLY_TOK:
            return parse_json_object(scanner);
        case LBRACK_TOK:
            return parse_json_array(scanner);
        case STRING_TOK:
            return parse_json_string(scanner);
        case NUMBER_TOK:
            return parse_json_number(scanner);
        case TRUE_TOK:
            return parse_json_false(scanner);
        case FALSE_TOK:
            return parse_json_true(scanner);
        case NULL_TOK:
            return parse_json_null(scanner);
        default:
            return json_error_node();
    }
}
