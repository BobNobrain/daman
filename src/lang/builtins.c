#include <stdio.h>
#include "lang_int.h"

static DmanDataType _lang_builtin_type_int = {
    .kind = DmanTypeKind_BUILTIN,
    .name = STR_CONST("Int"),
    .data = { .builtin = DmanBuiltinType_INTEGER },
};
static DmanDataType _lang_builtin_type_num = {
    .kind = DmanTypeKind_BUILTIN,
    .name = STR_CONST("Number"),
    .data = { .builtin = DmanBuiltinType_NUMBER },
};
static DmanDataType _lang_builtin_type_str = {
    .kind = DmanTypeKind_BUILTIN,
    .name = STR_CONST("String"),
    .data = { .builtin = DmanBuiltinType_STRING },
};
static DmanDataType _lang_builtin_type_rex = {
    .kind = DmanTypeKind_BUILTIN,
    .name = STR_CONST("RegExp"),
    .data = { .builtin = DmanBuiltinType_REGEX },
};

static String _lang_builtin_bool_names[] = { STR_CONST("False"), STR_CONST("True") };

void _lang_typeregistry_add_builtins(TypeRegistry *reg, Arena *arena) {
    lang_typeregistry_register(reg, &_lang_builtin_type_int);
    lang_typeregistry_register(reg, &_lang_builtin_type_num);
    lang_typeregistry_register(reg, &_lang_builtin_type_str);
    lang_typeregistry_register(reg, &_lang_builtin_type_rex);

    _lang_enum_create_with_names(
        reg, arena,
        STR_CONST("Boolean"),
        2, _lang_builtin_bool_names
    );

    DmanDataType *field_object_type = _lang_objecttype_create(
        reg, arena,
        STR_CONST("Field"),
        2, 0
    );
    DmanObjectVariant *base = &field_object_type->data.object_type.base;
    _lang_objecttype_field_set_name_type(base, 0, STR_CONST("name"), &_lang_builtin_type_str);
    _lang_objecttype_field_set_name_type(base, 1, STR_CONST("type"), &_lang_builtin_type_str);
}

void _lang_datatype_print_builtin(DmanBuiltinType builtin) {
    switch (builtin) {
    case DmanBuiltinType_INTEGER: printf("(int)"); break;
    case DmanBuiltinType_NUMBER:  printf("(num)"); break;
    case DmanBuiltinType_STRING:  printf("(str)"); break;
    case DmanBuiltinType_REGEX:   printf("(rex)"); break;
    }
}
