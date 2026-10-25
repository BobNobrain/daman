#include <stdlib.h>
#include <stdio.h>
#include "util.h"
#include "lang_int.h"

void lang_env_init(DmanEnv *env, Arena *arena) {
    env->arena = arena;
    env->data_types = arena_alloc(arena, sizeof(*env->data_types));
    env->symbols = arena_alloc(arena, sizeof(*env->symbols));

    _lang_typeregistry_init(env->data_types, arena);
    _lang_symtree_init(env->symbols, arena);

    _lang_env_add_prelude(env);
}

void lang_env_print(DmanEnv env) {
    _lang_typeregistry_print(env.data_types);
    _lang_symtree_print(env.symbols);
}

EnvEnumBuilder lang_env_build_enum(DmanEnv *env, String enum_name, size_t n_members) {
    DmanDataType *enum_type = _lang_enum_create(
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
                .type = SymbolType_ENUM_MEMBER,
                .name = member_name,
                .value = { .member = enum_member }
            }
        )
    );
}

EnvVariantBuilder lang_env_build_objecttype(DmanEnv *env, String object_name, size_t n_fields, size_t n_variants) {
    DmanDataType *object_type = _lang_objecttype_create(env->data_types, env->arena, object_name, n_fields, n_variants);
    LangSymbolTreeItem *item = _lang_symtree_insert(
        env->symbols, object_name,
        (LangSymbol) { .type = SymbolType_DATATYPE, .value = { .datatype = object_type } }
    );

    return (EnvVariantBuilder) {
        .env = env,
        .parent_symbol = item,
        .object_type = &object_type->data.object_type,
        .current_variant = &object_type->data.object_type.base,
        .n_fields_filled = 0,
        .n_variants_filled = 0,
    };
}

LangSymbol* lang_variant_builder_add_field(EnvVariantBuilder *builder, String field_name) {
    DmanObjectField *field = &builder->current_variant->fields[builder->n_fields_filled];
    field->name = field_name;
    builder->n_fields_filled += 1;

    LangSymbolTreeItem *field_symbol = _lang_symtree_alloc_item(
        builder->env->arena,
        (LangSymbol) {
            .type = SymbolType_OBJECT_FIELD,
            .name = field_name,
            .value = { .object_field = field },
        }
    );
    _lang_symtree_append_child(builder->parent_symbol, field_symbol);
    return &field_symbol->symbol;
}

EnvVariantBuilder lang_variant_builder_add_variant(
    EnvVariantBuilder *builder,
    String variant_name,
    size_t n_variant_fields,
    size_t n_variant_variants
) {
    if (n_variant_variants > 0 || builder->current_variant != &builder->object_type->base) {
        printf("NOT IMPLEMENTED: variants of variants");
        exit(EXIT_FAILURE);
    }

    DmanObjectVariant *variant = &builder->object_type->variants[builder->n_variants_filled];
    _lang_objecttype_init_variant(variant, builder->env->arena, variant_name, n_variant_fields);

    LangSymbolTreeItem *variant_symbol = _lang_symtree_alloc_item(
        builder->env->arena,
        (LangSymbol) {
            .type = SymbolType_OBJECT_VARIANT,
            .name = variant_name,
            .value = { .object_variant = variant },
        }
    );
    _lang_symtree_append_child(builder->parent_symbol, variant_symbol);

    return (EnvVariantBuilder) {
        .env = builder->env,
        .parent_symbol = variant_symbol,
        .object_type = builder->object_type,
        .current_variant = variant,
        .n_fields_filled = 0,
        .n_variants_filled = 0
    };
}
