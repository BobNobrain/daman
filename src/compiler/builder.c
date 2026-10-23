#include "lang.h"
#include "compiler_int.h"


void _compiler_collect_symbols(ASTNode *node, DmanEnv *env);

void _compiler_populate_env(Compiler *c, DmanEnv *env) {
    _compiler_collect_symbols(c->result, env);
    // TODO:
    // - walk again, now resolving all field types specifiers using the symbol table
}

void _compiler_collect_enum(ASTNode *enum_node, DmanEnv *env) {
    EnvEnumBuilder builder = lang_env_build_enum(
        env,
        enum_node->data.enum_decl.name,
        enum_node->data.enum_decl.n_members
    );

    for (ASTNode *it = enum_node->first_child; it != 0; it = it->next_sibling) {
        if (it->type != ASTNodeType_IDENT) { continue; }

        lang_enum_builder_add_member(&builder, it->data.ident.name);
    }
}

void _compiler_collect_symbols(ASTNode *node, DmanEnv *env) {
    if (_compiler_astnode_is_error(node)) { return; }

    bool should_process_children = true;

    switch (node->type) {
    case ASTNodeType_ENUM_DECL:
        _compiler_collect_enum(node, env);
        should_process_children = false;
    break;

    case ASTNodeType_OBJECT_DECL:
        // TODO
    break;

    default: break;
    }

    if (!should_process_children) { return; }

    for (ASTNode *it = node->first_child; it != 0; it = it->next_sibling) {
        _compiler_collect_symbols(it, env);
    }
}
