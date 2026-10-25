#include "lang.h"
#include "compiler_int.h"


typedef struct {
    DmanDataType **type_ptr;
    String type_name;
    DocumentRange location;
} ResolveTask;

ARRAY_DECL(ResolveTask)
ARRAY_METHODS_IMPL(_compiler_resolvetaskarray, ResolveTask)

void _compiler_collect_symbols(ASTNode *node, DmanEnv *env, ResolveTaskArray *resolve_tasks);
void _compiler_resolve_symbol(Compiler *c, DmanEnv *env, ResolveTask task);
String _compiler_unknown_symbol_msg(String symbol_name, Arena *arena);
String _compiler_wrong_symbol_type_msg(String symbol_name, Arena *arena);

void _compiler_populate_env(Compiler *c, DmanEnv *env) {
    ResolveTaskArray fields_to_resolve;
    _compiler_resolvetaskarray_init(&fields_to_resolve, 128, env->arena);
    _compiler_collect_symbols(c->result, env, &fields_to_resolve);

    for (size_t i = 0; i < fields_to_resolve.size; i++) {
        _compiler_resolve_symbol(c, env, fields_to_resolve.content[i]);
    }
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

void _compiler_collect_variant(
    ASTNode *variant_node,
    EnvVariantBuilder *builder,
    ResolveTaskArray *resolve_tasks
) {
    EnvVariantBuilder inner_builder;
    LangSymbol *field;

    for (ASTNode *it = variant_node->first_child; it != 0; it = it->next_sibling) {
        switch (it->type) {
        case ASTNodeType_OBJECT_FIELD_DECL:
            field = lang_variant_builder_add_field(builder, it->data.field_decl.name);
            _compiler_resolvetaskarray_append(resolve_tasks, (ResolveTask) {
                .location = it->basis_range,
                .type_name = it->data.field_decl.type_name,
                .type_ptr = &field->value.object_field->data_type,
            });
        break;

        case ASTNodeType_OBJECT_VARIANT_DECL:
            inner_builder = lang_variant_builder_add_variant(
                builder,
                it->data.variant_decl.name,
                it->data.variant_decl.n_fields,
                it->data.variant_decl.n_variants
            );
            _compiler_collect_variant(it, &inner_builder, resolve_tasks);
        break;

        default: break;
        }
    }
}

void _compiler_collect_object(ASTNode *object_node, DmanEnv *env, ResolveTaskArray *resolve_tasks) {
    EnvVariantBuilder builder = lang_env_build_objecttype(
        env,
        object_node->data.object_decl.name,
        object_node->data.object_decl.n_fields,
        object_node->data.object_decl.n_variants
    );

    _compiler_collect_variant(object_node, &builder, resolve_tasks);
}

void _compiler_collect_symbols(ASTNode *node, DmanEnv *env, ResolveTaskArray *resolve_tasks) {
    if (_compiler_astnode_is_error(node)) { return; }

    bool should_process_children = true;

    switch (node->type) {
    case ASTNodeType_ENUM_DECL:
        _compiler_collect_enum(node, env);
        should_process_children = false;
    break;

    case ASTNodeType_OBJECT_DECL:
        _compiler_collect_object(node, env, resolve_tasks);
        should_process_children = false;
    break;

    default: break;
    }

    if (!should_process_children) { return; }

    for (ASTNode *it = node->first_child; it != 0; it = it->next_sibling) {
        _compiler_collect_symbols(it, env, resolve_tasks);
    }
}

void _compiler_resolve_symbol(Compiler *c, DmanEnv *env, ResolveTask task) {
    LangSymbol *symbol = lang_env_resolve_name(*env, task.type_name);
    if (symbol == 0) {
        compiler_diagnostic_array_append(&c->diagnostics, (Diagnostic) {
            .location = task.location,
            .message = _compiler_unknown_symbol_msg(task.type_name, c->arena),
            .severity = DiagnosticSeverity_ERROR,
            .source = DiagnosticSource_TYPECHECK,
        });
        return;
    }

    switch (symbol->type) {
    case SymbolType_DATATYPE:
        *task.type_ptr = symbol->value.datatype;
    break;

    default:
        compiler_diagnostic_array_append(&c->diagnostics, (Diagnostic) {
            .location = task.location,
            .message = _compiler_wrong_symbol_type_msg(task.type_name, c->arena),
            .severity = DiagnosticSeverity_ERROR,
            .source = DiagnosticSource_TYPECHECK,
        });
        return;
    }
}

String _compiler_unknown_symbol_msg(String symbol_name, Arena *arena) {
    const String MSG_PREFIX = STR_CONST("could not resolve symbol '");
    const String MSG_SUFFIX = STR_CONST("'");

    StringBuilder sb;
    strb_init(&sb, arena, MSG_PREFIX.size + MSG_SUFFIX.size + symbol_name.size);
    strb_append(&sb, MSG_PREFIX);
    strb_append(&sb, symbol_name);
    strb_append(&sb, MSG_SUFFIX);
    return strb_render(&sb);
}

String _compiler_wrong_symbol_type_msg(String symbol_name, Arena *arena) {
    const String MSG_PREFIX = STR_CONST("'");
    const String MSG_SUFFIX = STR_CONST("' is not a type name");

    StringBuilder sb;
    strb_init(&sb, arena, MSG_PREFIX.size + MSG_SUFFIX.size + symbol_name.size);
    strb_append(&sb, MSG_PREFIX);
    strb_append(&sb, symbol_name);
    strb_append(&sb, MSG_SUFFIX);
    return strb_render(&sb);
}
