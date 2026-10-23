#include "compiler_int.h"
#include "grammar_int.h"

GRAMMAR_DECLARE( _compiler_g_toplevel_decl )
GRAMMAR_DECLARE( _compiler_g_enum_decl )
GRAMMAR_DECLARE( _compiler_g_object_decl )
GRAMMAR_DECLARE( _compiler_g_object_field )
GRAMMAR_DECLARE( _compiler_g_object_variant )
void _compiler_g_object_body(Parser *p, ASTObjectDeclData *data, ASTNode *parent);
void _compiler_g_skip_until_toplevel_keyword(Parser *p);


ASTNode* _compiler_g_dman_file(Parser *p) {
    ASTNode *file_root = _compiler_astnode_new(p->arena, ASTNodeType_DMAN_FILE);

    GRAMMAR_LOOP(next_token) {
        ASTNode *next_decl = _compiler_g_toplevel_decl(p);
        _compiler_astnode_append_child(file_root, next_decl);

        if (_compiler_astnode_is_error(next_decl)) {
            _compiler_g_skip_until_toplevel_keyword(p);
        }

        _compiler_g_skip_newlines(p);
    }

    return file_root;
}

ASTNode* _compiler_g_toplevel_decl(Parser *p) {
    GRAMMAR_ONEOF_START
    GRAMMAR_ONEOF_TRY( _compiler_g_enum_decl )
    GRAMMAR_ONEOF_TRY( _compiler_g_object_decl )

    return _compiler_astnode_new_error(p,
        STR_CONST("expected an enum declaration, or an object declaration")
    );
}

void _compiler_g_skip_until_toplevel_keyword(Parser *p) {
    Token *next = 0;
    while ((next = _compiler_parser_peek(p)) != 0) {
        if (next->type != TokenType_IDENT) {
            _compiler_parser_consume(p);
            continue;
        }

        switch (next->data.ident.keyword) {
        case Keyword_OBJECT:
        case Keyword_ENUM:
        case Keyword_ENCODING:
        case Keyword_TYPE:
            return;

        default: _compiler_parser_consume(p); break;
        }
    }
}

ASTNode* _compiler_g_enum_decl(Parser *p) {
    REQUIRE_KEYWORD(Keyword_ENUM, "expected a enum declaration")

    Token *enum_name = 0;
    REQUIRE_AND_SET_TOKEN(enum_name, TokenType_IDENT, "expected a name for the enum")

    REQUIRE_TOKEN(TokenType_BRACE_OPEN, "expected a '{'")
    _compiler_g_skip_newlines(p);
    ASTNode *enum_node = _compiler_astnode_new_from_token(p->arena, ASTNodeType_ENUM_DECL, *enum_name);
    enum_node->data.enum_decl.name = enum_name->data.ident.name;
    enum_node->data.enum_decl.n_members = 0;

    GRAMMAR_LOOP(next) {
        if (next->type == TokenType_BRACE_CLOSE) {
            break;
        }

        ASTNode *next_member = _compiler_g_identifier(p);
        _compiler_astnode_append_child(enum_node, next_member);
        if (!_compiler_astnode_is_error(next_member)) {
            enum_node->data.enum_decl.n_members += 1;
        }

        _compiler_g_separator(p);
        _compiler_g_skip_newlines(p);
    }

    Token *close_brace = 0;
    REQUIRE_AND_SET_TOKEN(close_brace, TokenType_BRACE_CLOSE, "expected a '}'")
    enum_node->full_range = compiler_document_range_span(enum_name->range, close_brace->range);

    _compiler_g_skip_newlines(p);

    return enum_node;
}

ASTNode* _compiler_g_object_decl(Parser *p) {
    REQUIRE_KEYWORD(Keyword_OBJECT, "expected an object declaration")

    Token *object_name = 0;
    REQUIRE_AND_SET_TOKEN(object_name, TokenType_IDENT, "expected a name for the enum")

    REQUIRE_TOKEN(TokenType_BRACE_OPEN, "expected a '{'")
    _compiler_g_skip_newlines(p);

    ASTNode *object_node = _compiler_astnode_new_from_token(p->arena, ASTNodeType_OBJECT_DECL, *object_name);
    object_node->data.object_decl.name = object_name->data.ident.name;

    _compiler_g_object_body(p, &object_node->data.object_decl, object_node);

    return object_node;
}

void _compiler_g_object_body(Parser *p, ASTObjectDeclData *data, ASTNode *parent) {
    data->n_fields = 0;
    data->n_variants = 0;

    GRAMMAR_LOOP(next) {
        if (next->type == TokenType_BRACE_CLOSE) {
            break;
        }

        ASTNode *next_member = 0;

        switch (next->type) {
        case TokenType_IDENT:
            next_member = _compiler_g_object_field(p);
            if (!_compiler_astnode_is_error(next_member)) {
                data->n_fields += 1;
            }
        break;

        case TokenType_OPERATOR:
            next_member = _compiler_g_object_variant(p);
            if (!_compiler_astnode_is_error(next_member)) {
                data->n_variants += 1;
            }
        break;

        default:
            next_member = _compiler_astnode_new_error(p,
                STR_CONST("expected a field declaration, or an object variant declaration")
            );
            next_member->basis_range = next->range;
            _compiler_g_skip_until_newline(p);
            next_member->full_range = compiler_document_range_span(next->range, _compiler_parser_current_range(p));
        break;
        }

        _compiler_astnode_append_child(parent, next_member);
        _compiler_g_skip_newlines(p);
    }

    Token *close_brace = _compiler_parser_consume_if(p, TokenType_BRACE_CLOSE);
    if (close_brace == 0) {
        ASTNode *err = _compiler_astnode_new_error(p, STR_CONST("expected a '}'"));
        _compiler_astnode_append_child(parent, err);
        return;
    }

    parent->full_range = compiler_document_range_span(parent->basis_range, close_brace->range);
    _compiler_g_skip_newlines(p);
}

ASTNode* _compiler_g_object_field(Parser *p) {
    Token *field_name, *field_type;
    REQUIRE_AND_SET_TOKEN(field_name, TokenType_IDENT, "expected field name")
    REQUIRE_AND_SET_TOKEN(field_type, TokenType_IDENT, "expected field type")

    ASTNode *field_node = _compiler_astnode_new_from_token(p->arena, ASTNodeType_OBJECT_FIELD_DECL, *field_name);
    field_node->full_range = compiler_document_range_span(field_name->range, field_type->range);
    field_node->data.field_decl = (ASTFieldDeclData) {
        .name = field_name->data.ident.name,
        .type_name = field_type->data.ident.name,
    };

    // ASTNode *field_type_node = _compiler_astnode_new_from_token(p->arena, ASTNodeType_OBJECT_FIELD_SPEC, *field_type);
    // _compiler_astnode_append_child(field_node, field_type_node);

    if (_compiler_parser_consume_if(p, TokenType_NEWLINE) == 0) {
        ASTNode *err = _compiler_astnode_new_error(p, STR_CONST("expected a newline"));
        _compiler_g_skip_until_newline(p);
        err->full_range = compiler_document_range_span(err->basis_range, _compiler_parser_current_range(p));
        _compiler_astnode_append_child(field_node, err);
    }

    return field_node;
}

ASTNode* _compiler_g_object_variant(Parser *p) {
    Token *tilde = 0;
    REQUIRE_AND_SET_TOKEN(tilde, TokenType_OPERATOR, "expected '~variant'")

    if (!str_eq(tilde->data.op, STR_CONST("~"))) {
        ASTNode* err = _compiler_astnode_new_error_ranged(p, STR_CONST("expected a newline"), tilde->range);
        return err;
    }

    REQUIRE_KEYWORD(Keyword_VARIANT, "expected 'variant' keyword")

    Token *variant_name = 0;
    REQUIRE_AND_SET_TOKEN(variant_name, TokenType_IDENT, "expected variant name")

    REQUIRE_TOKEN(TokenType_BRACE_OPEN, "expected a '{'")
    _compiler_g_skip_newlines(p);

    ASTNode *variant_decl = _compiler_astnode_new_from_token(p->arena, ASTNodeType_OBJECT_VARIANT_DECL, *variant_name);
    variant_decl->data.variant_decl.name = variant_name->data.ident.name;

    _compiler_g_object_body(p, &variant_decl->data.variant_decl, variant_decl);
    return variant_decl;
}
