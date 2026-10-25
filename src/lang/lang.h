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

extern void lang_datatype_print(DmanDataType *type);


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

#ifndef LANG_INTERNAL
typedef void SymbolTree;
typedef void LangSymbolTreeItem;
typedef void TypeRegistry;
#else
typedef struct SymbolTree SymbolTree;
#endif

typedef struct {
    Arena *arena;
    TypeRegistry *data_types;
    SymbolTree *symbols;
} DmanEnv;

extern void lang_env_init(DmanEnv *env, Arena *arena);
extern void lang_env_print(DmanEnv env);
extern WellKnownTypes lang_env_get_wkt(DmanEnv env);


typedef struct {
    DmanEnv *env;
    LangSymbolTreeItem *enum_symbol;
    size_t n_members_filled;
} EnvEnumBuilder;

extern EnvEnumBuilder lang_env_build_enum(DmanEnv *env, String fqn, size_t n_members);
extern void lang_enum_builder_add_member(EnvEnumBuilder *builder, String member_name);

typedef struct {
    DmanEnv *env;
    DmanObjectType *object_type;
    DmanObjectVariant *current_variant;
    LangSymbolTreeItem *parent_symbol;
    size_t n_fields_filled;
    size_t n_variants_filled;
} EnvVariantBuilder;

extern EnvVariantBuilder lang_env_build_objecttype(DmanEnv *env, String name, size_t n_fields, size_t n_variants);
extern LangSymbol* lang_variant_builder_add_field(EnvVariantBuilder *builder, String field_name);
extern EnvVariantBuilder lang_variant_builder_add_variant(
    EnvVariantBuilder *builder,
    String variant_name,
    size_t n_variant_fields,
    size_t n_variant_variants
);

#endif
