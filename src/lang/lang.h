#ifndef LANG_H
#define LANG_H

#include <stdlib.h>
#include "util.h"

typedef struct DmanDataType DmanDataType;

typedef enum {
    DmanTypeKind_BUILTIN,
    DmanTypeKind_ENUM,
    DmanTypeKind_ALIAS,
    DmanTypeKind_OBJECT,
    DmanTypeKind_MAP,
} DmanTypeKind;

typedef enum {
    DmanBuiltinType_INTEGER,
    DmanBuiltinType_NUMBER,
    DmanBuiltinType_STRING,
    DmanBuiltinType_REGEX,
} DmanBuiltinType;

typedef struct {
    String name;
    uint32_t id;
} DmanEnumMember;

typedef struct {
    DmanEnumMember *members;
    size_t n_members;
} DmanEnumType;

typedef struct {
    String name;
    size_t ordinal_number;
    DmanDataType *data_type;
} DmanObjectField;

typedef struct {
    String name;
    DmanObjectField *fields;
    size_t n_fields;
    // TODO: variants can have variants
} DmanObjectVariant;

typedef struct {
    DmanObjectVariant base;
    DmanObjectVariant *variants;
    size_t n_variants;
} DmanObjectType;

typedef struct {
    DmanDataType *from;
    DmanDataType *to;
} DmanMapType;

struct DmanDataType {
    union {
        DmanEnumType enum_type;
        DmanObjectType object_type;
        DmanMapType map_type;
        DmanBuiltinType builtin;
        DmanDataType *alias;
    } data;
    String name;
    DmanTypeKind kind;
};

ARRAY_DECL_NAMED(DmanDataType*, TypeArray)
ARRAY_METHODS_DECL_NAMED(_lang_typearray, DmanDataType*, TypeArray);

typedef struct {
    DmanDataType *integer_type;
    DmanDataType *number_type;
    DmanDataType *string_type;
    DmanDataType *regexp_type;
    DmanDataType *boolean_type;
    DmanDataType *basic_kind_type;
    DmanDataType *datatype_type;
    DmanDataType *field_type;
} WellKnownTypes;

#ifndef LANG_INTERNAL
typedef void TypeRegistry;
#else
typedef struct {
    WellKnownTypes wkt;
    TypeArray types;
    size_t next_free_enum_id;
} TypeRegistry;
#endif

extern void lang_typeregistry_register(TypeRegistry *reg, DmanDataType *type);
extern void lang_typeregistry_print(TypeRegistry *reg);
extern void lang_datatype_print(DmanDataType *type);

extern DmanDataType* lang_enum_create(TypeRegistry *reg, Arena *arena, String name, size_t n_members);


typedef struct {
    DmanDataType *from;
    DmanDataType *to;
} DmanEncoding;


typedef enum {
    SymbolType_NONE,
    SymbolType_DATATYPE,
    SymbolType_ENUM_MEMBER,
    SymbolType_OBJECT_VARIANT,
    SymbolType_OBJECT_FIELD,
    SymbolType_ENCODING,
} SymbolType;

typedef struct {
    union {
        DmanDataType *datatype;
        DmanEnumMember *member;
        DmanObjectVariant *object_variant;
        DmanObjectField *object_field;
        DmanEncoding *encoding;
    } value;
    String name;
    SymbolType type;
} LangSymbol;

typedef struct LangSymbolTreeItem {
    LangSymbol symbol;
    struct LangSymbolTreeItem *first_child;
    struct LangSymbolTreeItem *next_sibling;
} LangSymbolTreeItem;

typedef struct {
    Arena *arena;
    LangSymbolTreeItem *root;
} SymbolTree;

extern void lang_symtree_init(SymbolTree *tree, Arena *arena);
extern LangSymbol* lang_symtree_lookup(SymbolTree *tree, String fqn);
extern LangSymbol* lang_symtree_insert(SymbolTree *tree, String fqn, LangSymbol value);
extern void lang_symtree_print(SymbolTree *tree);


typedef struct {
    Arena *arena;
    TypeRegistry *data_types;
    SymbolTree *symbols;
} DmanEnv;

extern void lang_env_init(DmanEnv *env, Arena *arena);
extern void lang_env_print(DmanEnv env);

typedef struct {
    DmanEnv *env;
    LangSymbolTreeItem *enum_symbol;
    size_t n_members_filled;
} EnvEnumBuilder;

extern EnvEnumBuilder lang_env_build_enum(DmanEnv *env, String fqn, size_t n_members);
extern void lang_enum_builder_add_member(EnvEnumBuilder *builder, String member_name);

#endif
