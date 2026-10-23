#include "compiler_int.h"
#include "grammar_int.h"

ASTNode* _compiler_g_identifier(Parser *p) {
    Token *t = 0;
    REQUIRE_AND_SET_TOKEN(t, TokenType_IDENT, "expected an identifier");

    ASTNode *result = _compiler_astnode_new_from_token(p->arena, ASTNodeType_IDENT, *t);
    result->data.ident.name = t->data.ident.name;
    result->data.ident.references_to = 0;
    return result;
}

// consumes one separator (comma or newline)
bool _compiler_g_separator(Parser *p) {
    Token *next = _compiler_parser_peek(p);
    if (next == 0) { return false; }

    switch (next->type) {
    case TokenType_NEWLINE:
    case TokenType_COMMA:
        _compiler_parser_consume(p);
        return true;

    default: return false;
    }
}

// skips 0+ newlines
size_t _compiler_g_skip_newlines(Parser *p) {
    Token *next;
    size_t n_consumed = 0;
    while ((next = _compiler_parser_peek(p)) != 0) {
        if (next->type != TokenType_NEWLINE) {
            break;
        }

        _compiler_parser_consume(p);
        n_consumed += 1;
    }

    return n_consumed;
}

Token* _compiler_g_loop_end(Parser *p, Token *t) {
    Token *next = _compiler_parser_peek(p);

    if (next == t) {
        _compiler_parser_consume(p);
        return _compiler_parser_peek(p);
    }

    return next;
}

void _compiler_g_skip_until_newline(Parser *p) {
    Token *next;
    while ((next = _compiler_parser_peek(p)) != 0) {
        if (next->type == TokenType_NEWLINE) {
            break;
        }

        _compiler_parser_consume(p);
    }
}
