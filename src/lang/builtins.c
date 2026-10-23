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
static String _lang_builtin_basic_type_names[] = {
    STR_CONST("Int"), STR_CONST("Number"), STR_CONST("String"), STR_CONST("RexExp")
};

void _lang_typeregistry_init_datatype_type(TypeRegistry *reg, Arena *arena) {
    DmanObjectType *object_type = &reg->wkt.datatype_type->data.object_type;
    _lang_objecttype_field_set_name_type(&object_type->base, 0, STR_CONST("name"), reg->wkt.string_type);

    _lang_objecttype_init_variant(&object_type->variants[0], arena, STR_CONST("Basic"), 1);
    _lang_objecttype_field_set_name_type(&object_type->variants[0], 0, STR_CONST("kind"), reg->wkt.basic_kind_type);

    _lang_objecttype_init_variant(&object_type->variants[1], arena, STR_CONST("Alias"), 1);
    _lang_objecttype_field_set_name_type(&object_type->variants[1], 0, STR_CONST("over"), reg->wkt.datatype_type);

    // TODO: use object type for enum members
    DmanDataType *enum_members_type = _lang_maptype_create(reg, arena, reg->wkt.integer_type, reg->wkt.string_type);
    _lang_objecttype_init_variant(&object_type->variants[2], arena, STR_CONST("Enum"), 1);
    _lang_objecttype_field_set_name_type(&object_type->variants[2], 0, STR_CONST("members"), enum_members_type);

    _lang_objecttype_init_variant(&object_type->variants[3], arena, STR_CONST("Object"), 2);
    _lang_objecttype_field_set_name_type(&object_type->variants[3], 0, STR_CONST("fields"), reg->wkt.integer_type);
    _lang_objecttype_field_set_name_type(&object_type->variants[3], 1, STR_CONST("variants"), reg->wkt.integer_type);
}

void _lang_typeregistry_init_field_type(TypeRegistry *reg, Arena *arena) {
    (void) arena;

    DmanObjectVariant *field_base = &reg->wkt.field_type->data.object_type.base;
    _lang_objecttype_field_set_name_type(field_base, 0, STR_CONST("name"), reg->wkt.string_type);
    _lang_objecttype_field_set_name_type(field_base, 1, STR_CONST("type"), reg->wkt.datatype_type);
}

void _lang_typeregistry_add_builtins(TypeRegistry *reg, Arena *arena) {
    lang_typeregistry_register(reg, &_lang_builtin_type_int);
    reg->wkt.integer_type = &_lang_builtin_type_int;
    lang_typeregistry_register(reg, &_lang_builtin_type_num);
    reg->wkt.number_type = &_lang_builtin_type_num;
    lang_typeregistry_register(reg, &_lang_builtin_type_str);
    reg->wkt.string_type = &_lang_builtin_type_str;
    lang_typeregistry_register(reg, &_lang_builtin_type_rex);
    reg->wkt.regexp_type = &_lang_builtin_type_rex;

    reg->wkt.boolean_type = _lang_enum_create_with_names(
        reg, arena,
        STR_CONST("Boolean"),
        2, _lang_builtin_bool_names
    );

    reg->wkt.basic_kind_type = _lang_enum_create_with_names(
        reg, arena,
        STR_CONST("BasicType"),
        sizeof(_lang_builtin_basic_type_names) / sizeof(_lang_builtin_basic_type_names[0]),
        _lang_builtin_basic_type_names
    );

    reg->wkt.field_type = _lang_objecttype_create(
        reg, arena,
        STR_CONST("ObjectField"),
        2, 0
    );

    reg->wkt.datatype_type = _lang_objecttype_create(
        reg, arena,
        STR_CONST("DataType"),
        1, 4
    );

    _lang_typeregistry_init_datatype_type(reg, arena);
    _lang_typeregistry_init_field_type(reg, arena);
}

void _lang_datatype_print_builtin(DmanBuiltinType builtin) {
    switch (builtin) {
    case DmanBuiltinType_INTEGER: printf("(int)"); break;
    case DmanBuiltinType_NUMBER:  printf("(num)"); break;
    case DmanBuiltinType_STRING:  printf("(str)"); break;
    case DmanBuiltinType_REGEX:   printf("(rex)"); break;
    }
}
