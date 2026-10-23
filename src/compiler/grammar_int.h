#ifndef GRAMMAR_INT_H
#define GRAMMAR_INT_H

#include "compiler_int.h"


#define GRAMMAR_DECLARE(GRAMMAR) ASTNode* GRAMMAR(Parser *p);

#define GRAMMAR_ONEOF_START ParserRewindPoint saved_pos = { .input = p->input }; ASTNode *result = 0;
#define GRAMMAR_ONEOF_TRY(GRAMMAR) \
    result = GRAMMAR(p);                \
    if (!_compiler_astnode_is_error(result)) { \
        return result;                  \
    } else {                            \
        _compiler_parser_rewind(p, saved_pos); \
    }

#define REQUIRE_TOKEN(TOKEN_TYPE, ERR_MSG) \
    if (_compiler_parser_consume_if(p, TOKEN_TYPE) == 0) { return _compiler_astnode_new_error(p, STR_CONST(ERR_MSG)); }   \

#define REQUIRE_KEYWORD(KEYWORD_TYPE, ERR_MSG) \
    if (_compiler_parser_consume_keyword(p, KEYWORD_TYPE) == 0) {  \
        return _compiler_astnode_new_error(p, STR_CONST(ERR_MSG)); \
    }

#define REQUIRE_AND_SET_TOKEN(VAR, TOKEN_TYPE, ERR_MSG) \
    if ((VAR = _compiler_parser_consume_if(p, TOKEN_TYPE)) == 0) { \
        return _compiler_astnode_new_error(p, STR_CONST(ERR_MSG)); \
    }

#define GRAMMAR_LOOP(VAR) \
    for (Token *VAR = _compiler_parser_peek(p); VAR != 0; VAR = _compiler_g_loop_end(p, VAR))


GRAMMAR_DECLARE( _compiler_g_identifier )

extern Token* _compiler_g_loop_end(Parser *p, Token *t);
extern bool _compiler_g_separator(Parser *p);
extern size_t _compiler_g_skip_newlines(Parser *p);
extern void _compiler_g_skip_until_newline(Parser *p);

#endif
