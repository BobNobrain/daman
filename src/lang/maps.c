#include <stdio.h>
#include "util.h"
#include "lang_int.h"

void _lang_maptype_print(DmanMapType map_type, size_t indent) {
    printf("map ");

    _lang_datatype_print_inline_or_named(map_type.from, indent + 1);
    printf(" ");
    _lang_datatype_print_inline_or_named(map_type.to, indent + 1);
}

DmanDataType* _lang_maptype_create(TypeRegistry *reg, Arena *arena, DmanDataType *from, DmanDataType *to) {
    (void) reg;

    DmanDataType *datatype = arena_alloc(arena, sizeof(DmanDataType));
    datatype->kind = DmanTypeKind_MAP;
    datatype->name = STR_EMPTY;
    datatype->data.map_type = (DmanMapType) { .from = from, .to = to };
    return datatype;
}
