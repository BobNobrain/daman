#ifndef LANG_INTERNAL
#define LANG_INTERNAL

#include "lang.h"

extern void _lang_print_indent(size_t indent);

extern void _lang_typeregistry_init(TypeRegistry *reg, Arena *arena);
extern void _lang_typeregistry_add_builtins(TypeRegistry *reg, Arena *arena);
extern void _lang_datatype_print_builtin(DmanBuiltinType builtin);

extern DmanDataType* _lang_enum_create(TypeRegistry *reg, Arena *arena, String name, size_t n_members);
extern DmanDataType* _lang_enum_create_with_names(
    TypeRegistry *reg, Arena *arena,
    String name,
    size_t n_members,
    String *names
);
extern void _lang_enum_print(DmanEnumType enum_type);

extern void _lang_objecttype_print(DmanObjectType object, size_t indent);
extern DmanDataType* _lang_objecttype_create(
    TypeRegistry *reg, Arena *arena,
    String type_name,
    size_t n_base_fields,
    size_t n_variants
);
extern void _lang_objecttype_init_variant(DmanObjectVariant *variant, Arena *arena, String name, size_t n_fields);
extern void _lang_objecttype_field_set_name_type(
    DmanObjectVariant *variant,
    size_t index,
    String name,
    DmanDataType *data_type
);

#endif
