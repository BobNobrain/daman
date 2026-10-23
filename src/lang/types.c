#include <stdio.h>
#include "util.h"
#include "lang_int.h"

ARRAY_METHODS_IMPL_NAMED(_lang_typearray, DmanDataType*, TypeArray)

void lang_datatype_print(DmanDataType *type) {
    _lang_datatype_print_inline(type, 0);
    printf("\n");
}

void _lang_datatype_print_inline_or_named(DmanDataType *type, size_t indent) {
    if (type == 0) {
        printf("??");
        return;
    }

    if (!str_is_empty(type->name)) {
        printf(STR_FMT, STR_FMT_VAL(type->name));
        return;
    }

    _lang_datatype_print_inline(type, indent);
}

void _lang_datatype_print_inline(DmanDataType *type, size_t indent) {
    if (!str_is_empty(type->name)) {
        printf(STR_FMT" ", STR_FMT_VAL(type->name));
    }

    switch (type->kind) {
    case DmanTypeKind_BUILTIN:
        _lang_datatype_print_builtin(type->data.builtin);
    break;

    case DmanTypeKind_ALIAS:
        printf("(alias over "STR_FMT")", STR_FMT_VAL(type->data.alias->name));
    break;

    case DmanTypeKind_ENUM:
        _lang_enum_print(type->data.enum_type);
    break;

    case DmanTypeKind_OBJECT:
        _lang_objecttype_print(type->data.object_type, indent);
    break;

    case DmanTypeKind_MAP:
        _lang_maptype_print(type->data.map_type, indent);
    break;
    }
}
