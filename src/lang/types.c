#include <stdio.h>
#include "util.h"
#include "lang_int.h"

ARRAY_METHODS_IMPL_NAMED(_lang_typearray, DmanDataType*, TypeArray)

void lang_datatype_print(DmanDataType *type, size_t indent) {
    _lang_print_indent(indent);

    printf("type: "STR_FMT" ", STR_FMT_VAL(type->name));

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
    }

    printf("\n");
}
