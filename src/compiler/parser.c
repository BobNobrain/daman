#include "compiler_int.h"

void _compiler_parser_init(Parser *parser, Compiler *c) {
    parser->arena = c->arena;
    parser->diagnostics = &c->diagnostics;
    parser->root = 0;
    parser->source = c->source;
    parser->input = (TokenSlice) { 0 };
    parser->input_original = (TokenSlice) { 0 };
}

void _compiler_parser_parse(Parser *p, TokenSlice input, ParserGrammar grammar) {
    p->input_original = input;
    p->input = input;

    p->root = grammar(p);
    _compiler_parser_collect_errors(p, p->root);

    if (p->input.size > 0) {
        _compiler_parser_add_diagnostic(p, (Diagnostic) {
            .location = p->input.content[0].range,
            .message = STR_CONST("expected end of file"),
            .severity = DiagnosticSeverity_ERROR,
            .source = DiagnosticSource_SYNTAX,
        });
    }
}

Token* _compiler_parser_consume(Parser *p) {
    if (p->input.size == 0) { return 0; }

    Token *result = &p->input.content[0];
    p->input = _compiler_tokenslice_slice(p->input, 1, p->input.size);
    return result;
}

Token* _compiler_parser_peek(Parser *p) {
    if (p->input.size == 0) { return 0; }
    return &p->input.content[0];
}

Token* _compiler_parser_consume_if(Parser *p, TokenType type) {
    if (p->input.size == 0) { return 0; }

    Token *result = &p->input.content[0];
    if (result->type != type) { return 0; }

    p->input = _compiler_tokenslice_slice(p->input, 1, p->input.size);
    return result;

}

Token* _compiler_parser_consume_keyword(Parser *p, Keyword kw) {
    if (p->input.size == 0) { return 0; }

    Token *result = &p->input.content[0];
    if (result->type != TokenType_IDENT) { return 0; }
    if (result->data.ident.keyword != kw) { return 0; }

    p->input = _compiler_tokenslice_slice(p->input, 1, p->input.size);
    return result;
}

void _compiler_parser_rewind(Parser *p, ParserRewindPoint to) {
    p->input = to.input;
}

DocumentRange _compiler_parser_current_range(Parser *p) {
    if (p->input.size == 0) {
        if (p->input_original.size == 0) {
            return (DocumentRange) { 0 };
        }

        return _compiler_tokenslice_at(p->input_original, p->input_original.size - 1)->range;
    }

    return p->input.content[0].range;
}

void _compiler_parser_add_diagnostic(Parser *p, Diagnostic d) {
    d.source = DiagnosticSource_SYNTAX;
    compiler_diagnostic_array_append(p->diagnostics, d);
}

void _compiler_parser_collect_errors(Parser *p, ASTNode *node) {
    if (node == 0) {
        _compiler_parser_add_diagnostic(p, (Diagnostic) {
            .location = (DocumentRange) { 0 },
            .message = STR_CONST("null node found"),
            .severity = DiagnosticSeverity_ERROR,
        });
        return;
    }

    if (node->type == ASTNodeType_SYNTAX_ERROR) {
        _compiler_parser_add_diagnostic(p, (Diagnostic) {
            .location = node->basis_range,
            .message = node->data.error,
            .severity = DiagnosticSeverity_ERROR,
        });
    }

    ASTNode *it = node->first_child;
    while (it != 0) {
        _compiler_parser_collect_errors(p, it);
        it = it->next_sibling;
    }
}

ASTNode* _compiler_run_parsing(Compiler *c, TokenSlice input, ParserGrammar grammar) {
    Parser p = { 0 };
    _compiler_parser_init(&p, c);
    _compiler_parser_parse(&p, input, grammar);
    return p.root;
}
