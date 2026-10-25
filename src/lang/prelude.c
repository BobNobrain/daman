#include "lang_int.h"

void _lang_env_register_primitive(DmanEnv *env, DmanDataType *type);
void _lang_env_register_reflect_types(DmanEnv *env);
void _lang_env_register_field_with_type(EnvVariantBuilder *builder, String name, DmanDataType *type);

void _lang_env_add_prelude(DmanEnv *env) {
    _lang_env_register_primitive(env, _lang_builtin_get_int());
    _lang_env_register_primitive(env, _lang_builtin_get_num());
    _lang_env_register_primitive(env, _lang_builtin_get_str());
    _lang_env_register_primitive(env, _lang_builtin_get_rex());

    env->data_types->wkt.integer_type = _lang_builtin_get_int();
    env->data_types->wkt.number_type = _lang_builtin_get_num();
    env->data_types->wkt.string_type = _lang_builtin_get_str();
    env->data_types->wkt.regexp_type = _lang_builtin_get_rex();

    EnvEnumBuilder boolean_builder = lang_env_build_enum(env, STR_CONST("Boolean"), 2);
    lang_enum_builder_add_member(&boolean_builder, STR_CONST("False"));
    lang_enum_builder_add_member(&boolean_builder, STR_CONST("True"));
    env->data_types->wkt.boolean_type = boolean_builder.enum_symbol->symbol.value.datatype;

    _lang_env_register_reflect_types(env);
}

void _lang_env_register_reflect_types(DmanEnv *env) {
    EnvEnumBuilder basic_builder = lang_env_build_enum(env, STR_CONST("BasicType"), 4);
    env->data_types->wkt.basic_kind_type = basic_builder.enum_symbol->symbol.value.datatype;
    lang_enum_builder_add_member(&basic_builder, STR_CONST("Int"));
    lang_enum_builder_add_member(&basic_builder, STR_CONST("Number"));
    lang_enum_builder_add_member(&basic_builder, STR_CONST("String"));
    lang_enum_builder_add_member(&basic_builder, STR_CONST("RegExp"));

    EnvVariantBuilder dt_builder = lang_env_build_objecttype(env, STR_CONST("DataType"), 1, 4);
    _lang_env_register_field_with_type(&dt_builder, STR_CONST("name"), env->data_types->wkt.string_type);
    env->data_types->wkt.datatype_type = dt_builder.parent_symbol->symbol.value.datatype;

    EnvVariantBuilder field_builder;
    field_builder = lang_env_build_objecttype(env, STR_CONST("ObjectField"), 2, 0);
    env->data_types->wkt.field_type = field_builder.parent_symbol->symbol.value.datatype;

    _lang_env_register_field_with_type(&field_builder, STR_CONST("name"), env->data_types->wkt.string_type);
    _lang_env_register_field_with_type(&field_builder, STR_CONST("type"), env->data_types->wkt.datatype_type);

    EnvVariantBuilder dt_variant_builder = lang_variant_builder_add_variant(&dt_builder, STR_CONST("Basic"), 1, 0);
    _lang_env_register_field_with_type(&dt_variant_builder, STR_CONST("kind"), env->data_types->wkt.basic_kind_type);

    dt_variant_builder = lang_variant_builder_add_variant(&dt_builder, STR_CONST("Alias"), 1, 0);
    _lang_env_register_field_with_type(&dt_variant_builder, STR_CONST("over"), env->data_types->wkt.datatype_type);

    dt_variant_builder = lang_variant_builder_add_variant(&dt_builder, STR_CONST("Enum"), 1, 0);
    _lang_env_register_field_with_type(&dt_variant_builder, STR_CONST("members"), 0);

    dt_variant_builder = lang_variant_builder_add_variant(&dt_builder, STR_CONST("Object"), 2, 0);
    _lang_env_register_field_with_type(&dt_variant_builder, STR_CONST("fields"), 0);
    _lang_env_register_field_with_type(&dt_variant_builder, STR_CONST("variants"), 0);
}

void _lang_env_register_primitive(DmanEnv *env, DmanDataType *type) {
    _lang_typeregistry_register(env->data_types, type);
    _lang_symtree_insert(env->symbols, type->name, (LangSymbol) {
        .type = SymbolType_DATATYPE,
        .name = type->name,
        .value = { .datatype = type },
    });
}

void _lang_env_register_field_with_type(EnvVariantBuilder *builder, String name, DmanDataType *type) {
    LangSymbol *field_symbol = lang_variant_builder_add_field(builder, name);
    field_symbol->value.object_field->data_type = type;
}
