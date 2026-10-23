#include "compiler_int.h"

ASTNode* _compiler_astnode_new(Arena *arena, ASTNodeType type) {
    ASTNode *new_node = arena_alloc(arena, sizeof(*new_node));

    *new_node = (ASTNode) { 0 };
    new_node->type = type;

    return new_node;
}
ASTNode* _compiler_astnode_new_from_token(Arena *arena, ASTNodeType type, Token token) {
    ASTNode* node = _compiler_astnode_new(arena, type);
    node->basis_range = token.range;
    node->full_range = token.range;
    return node;
}

ASTNode* _compiler_astnode_new_error(Parser *parser, String msg) {
    ASTNode *e = _compiler_astnode_new(parser->arena, ASTNodeType_SYNTAX_ERROR);
    e->data.error = msg;
    e->full_range = _compiler_parser_current_range(parser);
    e->basis_range = _compiler_parser_current_range(parser);
    return e;
}

ASTNode* _compiler_astnode_new_error_ranged(Parser *parser, String msg, DocumentRange range) {
    ASTNode *e = _compiler_astnode_new(parser->arena, ASTNodeType_SYNTAX_ERROR);
    e->data.error = msg;
    e->full_range = range;
    e->basis_range = range;
    return e;
}


void _compiler_astnode_append_child(ASTNode *parent, ASTNode *child) {
    if (parent->first_child == 0) {
        parent->first_child = child;
        return;
    }

    ASTNode *last = _compiler_astnode_find_last_child(parent);
    last->next_sibling = child;
}

size_t _compiler_astnode_count_children(ASTNode *parent) {
    ASTNode *it = parent->first_child;
    size_t result = 0;
    while (it) {
        result += 1;
        it = it->next_sibling;
    }
    return result;
}

ASTNode* _compiler_astnode_find_last_child(ASTNode *parent) {
    if (parent->first_child == 0) { return 0; }

    ASTNode *it = parent->first_child;
    while (it->next_sibling != 0) {
        it = it->next_sibling;
    }

    return it;
}

void compiler_astnode_print(ASTNode *node, size_t indent) {
    if (node == 0) {
        printf("<null>\n");
        return;
    }

    for (size_t i = 0; i < indent; i++) {
        printf("  ");
    }

    switch (node->type) {
    case ASTNodeType_SYNTAX_ERROR:
        printf(
            "ERROR (%zu:%zu-%zu:%zu): " STR_FMT,
            node->full_range.start.line,
            node->full_range.start.column,
            node->full_range.end.line,
            node->full_range.end.column,
            STR_FMT_VAL(node->data.error)
        );
    break;

    case ASTNodeType_LINT: printf("INT %d", node->data.l_int); break;
    case ASTNodeType_LNUM: printf("NUM %f", node->data.l_num); break;
    case ASTNodeType_IDENT: printf("IDENT "STR_FMT, STR_FMT_VAL(node->data.ident.name)); break;

    case ASTNodeType_IDENT_NESTED: printf("NESTED IDENT "); break;
    case ASTNodeType_ENUM_DECL: printf("ENUM "STR_FMT, STR_FMT_VAL(node->data.enum_decl.name)); break;

    case ASTNodeType_OBJECT_DECL:
        printf(
            "OBJECT "STR_FMT" (%zu fields, %zu variants)",
            STR_FMT_VAL(node->data.object_decl.name),
            node->data.object_decl.n_fields,
            node->data.object_decl.n_variants
        );
    break;

    case ASTNodeType_OBJECT_FIELD_DECL:
        printf(
            "FIELD "STR_FMT" "STR_FMT,
            STR_FMT_VAL(node->data.field_decl.name),
            STR_FMT_VAL(node->data.field_decl.type_name)
        );
    break;

    case ASTNodeType_OBJECT_FIELD_SPEC: printf("FIELD SPEC "); break;

    case ASTNodeType_OBJECT_VARIANT_DECL:
        printf(
            "VARIANT "STR_FMT" (%zu fields, %zu variants)",
            STR_FMT_VAL(node->data.variant_decl.name),
            node->data.variant_decl.n_fields,
            node->data.variant_decl.n_variants
        );
    break;

    case ASTNodeType_DMAN_FILE: printf(".DMAN FILE"); break;

    default: printf("UNKNOWN"); break;
    }

    printf("\n");

    ASTNode *it = node->first_child;
    while (it != 0) {
        compiler_astnode_print(it, indent + 1);
        it = it->next_sibling;
    }
}

bool _compiler_astnode_is_error(ASTNode *node) {
    return node == 0 || node->type == ASTNodeType_SYNTAX_ERROR;
}

void _compiler_astnode_set_range_span_children(ASTNode *node) {
    if (!node->first_child) {
        node->full_range = node->basis_range;
        return;
    }

    ASTNode *last = _compiler_astnode_find_last_child(node);
    if (last == node->first_child) {
        node->full_range = node->first_child->full_range;
        return;
    }

    node->full_range = compiler_document_range_span(node->first_child->full_range, last->full_range);
}
