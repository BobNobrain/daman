#ifndef LANG_INTERNAL
#define LANG_INTERNAL

#include "util.h"

typedef struct LangSymbolTreeItem LangSymbolTreeItem;

#include "lang.h"

struct SymbolTree {
    Arena *arena;
    LangSymbolTreeItem *root;
};

struct LangSymbolTreeItem {
    LangSymbol symbol;
    struct LangSymbolTreeItem *first_child;
    struct LangSymbolTreeItem *next_sibling;
};

extern void _lang_print_indent(size_t indent);

extern void _lang_typeregistry_init(TypeRegistry *reg, Arena *arena);
extern void _lang_datatype_print_builtin(DmanBuiltinType builtin);
extern DmanDataType* _lang_builtin_get_int();
extern DmanDataType* _lang_builtin_get_num();
extern DmanDataType* _lang_builtin_get_str();
extern DmanDataType* _lang_builtin_get_rex();
extern void _lang_datatype_print_inline_or_named(DmanDataType *type, size_t indent);
extern void _lang_datatype_print_inline(DmanDataType *type, size_t indent);

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

extern DmanDataType* _lang_maptype_create(TypeRegistry *reg, Arena *arena, DmanDataType *from, DmanDataType *to);
extern void _lang_maptype_print(DmanMapType map_type, size_t indent);

extern void _lang_symtree_init(SymbolTree *tree, Arena *arena);
extern LangSymbol* _lang_symtree_lookup(SymbolTree *tree, String fqn);
extern void _lang_symtree_print(SymbolTree *tree);
extern LangSymbolTreeItem* _lang_symtree_insert(SymbolTree *tree, String fqn, LangSymbol symbol);
extern void _lang_symtree_append_child(LangSymbolTreeItem *parent, LangSymbolTreeItem *child);
extern LangSymbolTreeItem* _lang_symtree_alloc_item(Arena *arena, LangSymbol symbol);

extern void _lang_env_add_prelude(DmanEnv *env);

#endif
