#include <stdio.h>
#include "util.h"
#include "lang_int.h"

void _lang_objecttype_print_field(DmanObjectField field, size_t indent) {
    printf("(%zu) "STR_FMT" ", field.ordinal_number, STR_FMT_VAL(field.name));
    _lang_datatype_print_inline_or_named(field.data_type, indent + 1);
}

void _lang_objecttype_print_fields(DmanObjectVariant *variant, size_t indent) {
    for (size_t i = 0; i < variant->n_fields; i++) {
        _lang_print_indent(indent);
        _lang_objecttype_print_field(variant->fields[i], indent);
        printf("\n");
    }
}

void _lang_objecttype_print(DmanObjectVariant object, size_t indent) {
    printf("(object, %zu fields, %zu variants) {\n", object.n_fields, object.n_variants);

    _lang_objecttype_print_fields(&object, indent + 1);

    for (size_t i = 0; i < object.n_variants; i++) {
        DmanObjectVariant variant = object.variants[i];
        _lang_print_indent(indent + 1);
        printf("~variant "STR_FMT" ", STR_FMT_VAL(variant.name));
        _lang_objecttype_print(variant, indent + 2);
        printf("\n");
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

void _lang_objecttype_init_variant(
    DmanObjectVariant *variant,
    Arena *arena,
    String name,
    size_t n_fields,
    size_t n_variants
) {
    variant->name = name;
    variant->fields = 0;
    variant->n_fields = n_fields;
    variant->variants = 0;
    variant->n_variants = n_variants;

    if (n_variants > 0) {
        variant->variants = arena_alloc(arena, sizeof(variant->variants[0]) * n_variants);

        for (size_t i = 0; i < n_variants; i++) {
            _lang_objecttype_init_variant(&variant->variants[i], arena, STR_EMPTY, 0, 0);
        }
    }

    if (n_fields > 0) {
        variant->fields = arena_alloc(arena, sizeof(variant->fields[0]) * n_fields);

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
    result->data.object_type = (DmanObjectVariant) {
        .name = type_name,
        .n_fields = n_base_fields,
        .fields = 0,
        .n_variants = n_variants,
        .variants = 0,
    };

    _lang_objecttype_init_variant(&result->data.object_type, arena, type_name, n_base_fields, n_variants);
    _lang_typeregistry_register(reg, result);
    return result;
}
