#include <stdio.h>
#include "lang_int.h"

DmanDataType* _lang_enum_create(TypeRegistry *reg, Arena *arena, String name, size_t n_members) {
    DmanDataType *data_type = arena_alloc(arena, sizeof(DmanDataType));

    data_type->kind = DmanTypeKind_ENUM;
    data_type->name = name;
    data_type->data.enum_type.n_members = n_members;
    data_type->data.enum_type.members = arena_alloc(arena, n_members * sizeof(data_type->data.enum_type.members[0]));

    for (size_t i = 0; i < n_members; i++) {
        data_type->data.enum_type.members[i].id = reg->next_free_enum_id + i;
        data_type->data.enum_type.members[i].name = STR_EMPTY;
    }

    _lang_typeregistry_register(reg, data_type);
    return data_type;
}

void _lang_enum_print(DmanEnumType enum_type) {
    printf("(enum, %zu members) { ", enum_type.n_members);

    for (size_t i = 0; i < enum_type.n_members; i++) {
        if (i > 0) { printf(", "); }

        printf(STR_FMT"(id=%u)", STR_FMT_VAL(enum_type.members[i].name), enum_type.members[i].id);
    }

    printf(" }");
}
