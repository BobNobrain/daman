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

DmanDataType* _lang_builtin_get_int() { return &_lang_builtin_type_int; }
DmanDataType* _lang_builtin_get_num() { return &_lang_builtin_type_num; }
DmanDataType* _lang_builtin_get_str() { return &_lang_builtin_type_str; }
DmanDataType* _lang_builtin_get_rex() { return &_lang_builtin_type_rex; }

void _lang_datatype_print_builtin(DmanBuiltinType builtin) {
    switch (builtin) {
    case DmanBuiltinType_INTEGER: printf("(int)"); break;
    case DmanBuiltinType_NUMBER:  printf("(num)"); break;
    case DmanBuiltinType_STRING:  printf("(str)"); break;
    case DmanBuiltinType_REGEX:   printf("(rex)"); break;
    }
}
