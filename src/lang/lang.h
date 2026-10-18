#ifndef LANG_H
#define LANG_H

#include <stdlib.h>
#include "util.h"

typedef struct DmanDataType DmanDataType;

typedef enum {
    DmanTypeKind_ENUM,
    DmanTypeKind_OBJECT,
    DmanTypeKind_ALIAS,
    DmanTypeKind_BUILTIN,
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
} DmanObjectVariant;

typedef struct {
    DmanObjectVariant base;
    DmanObjectVariant *variants;
    size_t n_variants;
} DmanObjectType;

struct DmanDataType {
    union {
        DmanEnumType enum_type;
        DmanObjectType object_type;
        DmanBuiltinType builtin;
        DmanDataType *alias;
    } data;
    String name;
    DmanTypeKind kind;
};

ARRAY_DECL_NAMED(DmanDataType*, TypeArray)
ARRAY_METHODS_DECL_NAMED(_lang_typearray, DmanDataType*, TypeArray);

#ifndef LANG_INTERNAL
typedef void TypeRegistry;
#else
typedef struct {
    TypeArray types;
    size_t next_free_enum_id;
} TypeRegistry;
#endif

// extern DmanDataType* lang_typeregistry_find_type(TypeRegistry *reg, String name);
extern void lang_typeregistry_register(TypeRegistry *reg, DmanDataType *type);
extern void lang_typeregistry_print(TypeRegistry *reg);
extern void lang_datatype_print(DmanDataType *type, size_t indent);

typedef struct {
    DmanDataType *from;
    DmanDataType *to;
} DmanEncoding;

typedef struct {
    Arena *arena;
    TypeRegistry *data_types;
} DmanEnv;

extern void lang_env_init(DmanEnv *env, Arena *arena);
extern void lang_env_print(DmanEnv env);

#endif
