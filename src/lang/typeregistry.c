#include <stdio.h>
#include "util.h"
#include "lang_int.h"

void lang_typeregistry_register(TypeRegistry *reg, DmanDataType *type) {
    _lang_typearray_append(&reg->types, type);

    if (type->kind == DmanTypeKind_ENUM) {
        reg->next_free_enum_id += type->data.enum_type.n_members;
    }
}

void _lang_typeregistry_init(TypeRegistry *reg, Arena *arena) {
    _lang_typearray_init(&reg->types, 64, arena);
    reg->next_free_enum_id = 1; // reserving 0 for error state
}

void lang_typeregistry_print(TypeRegistry *reg) {
    printf("TypeRegistry (.next_id = %zu, %zu types)\n", reg->next_free_enum_id, reg->types.size);

    for (size_t i = 0; i < reg->types.size; i++) {
        lang_datatype_print(reg->types.content[i], 0);
    }
}
