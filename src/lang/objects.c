#include <stdio.h>
#include "util.h"
#include "lang_int.h"

void _lang_objecttype_print_field(DmanObjectField field, size_t indent) {
    printf("(%zu) "STR_FMT" ", field.ordinal_number, STR_FMT_VAL(field.name));
    _lang_datatype_print_inline_or_named(field.data_type, indent + 1);
}

void _lang_objecttype_print_variant(DmanObjectVariant *variant, size_t indent) {
    for (size_t i = 0; i < variant->n_fields; i++) {
        _lang_print_indent(indent);
        _lang_objecttype_print_field(variant->fields[i], indent);
        printf("\n");
    }
}

void _lang_objecttype_print(DmanObjectType object, size_t indent) {
    printf("(object, %zu base fields, %zu variants) {\n", object.base.n_fields, object.n_variants);

    _lang_objecttype_print_variant(&object.base, indent + 1);

    for (size_t i = 0; i < object.n_variants; i++) {
        DmanObjectVariant *variant = &object.variants[i];
        _lang_print_indent(indent + 1);
        printf("~variant "STR_FMT" (%zu fields) {\n", STR_FMT_VAL(variant->name), variant->n_fields);
        _lang_objecttype_print_variant(variant, indent + 2);
        _lang_print_indent(indent + 1);
        printf("}\n");
    }

    _lang_print_indent(indent);
    printf("}");
}

void _lang_objecttype_field_set_name_type(
    DmanObjectVariant *variant,
    size_t index,
    String name,
    DmanDataType *data_type
) {
    variant->fields[index].name = name;
    variant->fields[index].data_type = data_type;
}

void _lang_objecttype_init_variant(DmanObjectVariant *variant, Arena *arena, String name, size_t n_fields) {
    variant->n_fields = n_fields;
    variant->name = name;
    variant->fields = 0;

    if (n_fields > 0) {
        variant->fields = arena_alloc(arena, sizeof(DmanObjectField) * n_fields);

        for (size_t i = 0; i < n_fields; i++) {
            variant->fields[i] = (DmanObjectField) {
                .name = STR_EMPTY,
                .data_type = 0,
                .ordinal_number = i,
            };
        }
    }
}

DmanDataType* _lang_objecttype_create(
    TypeRegistry *reg, Arena *arena,
    String type_name,
    size_t n_base_fields,
    size_t n_variants
) {
    DmanDataType *result = arena_alloc(arena, sizeof(DmanDataType));
    result->kind = DmanTypeKind_OBJECT;
    result->name = type_name;
    result->data.object_type = (DmanObjectType) {
        .base = {
            .name = type_name,
            .n_fields = n_base_fields,
            .fields = 0,
        },
        .n_variants = n_variants,
        .variants = 0,
    };

    if (n_base_fields > 0) {
        _lang_objecttype_init_variant(&result->data.object_type.base, arena, type_name, n_base_fields);
    }

    if (n_variants > 0) {
        result->data.object_type.variants = arena_alloc(arena, sizeof(DmanObjectVariant) * n_variants);

        for (size_t i = 0; i < n_variants; i++) {
            _lang_objecttype_init_variant(&result->data.object_type.variants[i], arena, STR_EMPTY, 0);
        }
    }

    _lang_typeregistry_register(reg, result);
    return result;
}
