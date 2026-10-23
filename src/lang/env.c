#include "util.h"
#include "lang_int.h"

void lang_env_init(DmanEnv *env, Arena *arena) {
    env->arena = arena;
    env->data_types = arena_alloc(arena, sizeof(*env->data_types));
    env->symbols = arena_alloc(arena, sizeof(*env->symbols));

    _lang_typeregistry_init(env->data_types, arena);
    _lang_typeregistry_add_builtins(env->data_types, arena);
    lang_symtree_init(env->symbols, arena);
}

void lang_env_print(DmanEnv env) {
    lang_typeregistry_print(env.data_types);
    lang_symtree_print(env.symbols);
}

EnvEnumBuilder lang_env_build_enum(DmanEnv *env, String enum_name, size_t n_members) {
    DmanDataType *enum_type = lang_enum_create(
        env->data_types,
        env->arena,
        enum_name,
        n_members
    );

    LangSymbolTreeItem *item = _lang_symtree_insert(
        env->symbols, enum_name,
        (LangSymbol) { .type = SymbolType_DATATYPE, .value = { .datatype = enum_type } }
    );

    return (EnvEnumBuilder) { .enum_symbol = item, .env = env, .n_members_filled = 0 };
}

void lang_enum_builder_add_member(EnvEnumBuilder *builder, String member_name) {
    DmanDataType *enum_datatype = builder->enum_symbol->symbol.value.datatype;
    DmanEnumMember *enum_member = &enum_datatype->data.enum_type.members[builder->n_members_filled];

    enum_member->name = member_name;
    builder->n_members_filled += 1;

    _lang_symtree_append_child(
        builder->enum_symbol,
        _lang_symtree_alloc_item(
            builder->env->arena,
            (LangSymbol) {
                .name = member_name,
                .type = SymbolType_ENUM_MEMBER,
                .value = { .member = enum_member }
            }
        )
    );
}

