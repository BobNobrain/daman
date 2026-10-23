#ifndef COMPILER_H
#define COMPILER_H

#include "util.h"
#include "lang.h"

typedef struct Compiler Compiler;
typedef struct ASTNode ASTNode;


typedef struct DocumentPos {
    size_t caret;
    size_t line;
    size_t column;
} DocumentPos;

extern DocumentPos compiler_document_pos_zero();
extern void compiler_document_pos_track(DocumentPos *pos, size_t chars);
extern void compiler_document_pos_line_break(DocumentPos *pos);

typedef struct DocumentRange {
    DocumentPos start;
    DocumentPos end;
} DocumentRange;

extern DocumentRange compiler_document_range(DocumentPos start, size_t length);
extern DocumentRange compiler_document_range_span(DocumentRange from, DocumentRange to);
extern int compiler_document_range_length(DocumentRange range);
extern String compiler_document_substring(String source, DocumentRange range);

typedef enum DiagnosticSeverity {
    DiagnosticSeverity_ERROR,
    DiagnosticSeverity_WARN,
    DiagnosticSeverity_INFO,
} DiagnosticSeverity;

typedef enum DiagnosticSource {
    DiagnosticSource_OTHER,
    DiagnosticSource_SYNTAX,
    DiagnosticSource_TYPECHECK,
} DiagnosticSource;

typedef struct Diagnostic {
    DocumentRange location;
    String message;
    DiagnosticSource source;
    DiagnosticSeverity severity;
} Diagnostic;

extern void compiler_diagnostic_print(Diagnostic d, String source_doc, Arena *arena);

ARRAY_DECL(Diagnostic)
ARRAY_METHODS_DECL(compiler_diagnostic_array, Diagnostic)


typedef uint8_t OperatorPriority;
#define OPERATOR_PRIORITY_MIN 0
#define OPERATOR_PRIORITY_MAX 255
typedef enum {
    OperatorType_INVALID,
    OperatorType_BINARY_NOASSOC,
    OperatorType_BINARY_LEFT,
    OperatorType_BINARY_RIGHT,
    OperatorType_UNARY_NOASSOC,
    OperatorType_UNARY_LEFT,
    OperatorType_UNARY_RIGHT,
} OperatorType;

typedef int OperatorVariant;
enum OperatorVariant {
    OperatorVariant_UNKNOWN,
    OperatorVariant_UNARY_NOT,
    OperatorVariant_UNARY_MINUS,
    OperatorVariant_UNARY_PLUS,
    OperatorVariant_UNARY_CAST,
    OperatorVariant_BINARY_POWER,
    OperatorVariant_BINARY_MUL,
    OperatorVariant_BINARY_DIV,
    OperatorVariant_BINARY_REM,
    OperatorVariant_BINARY_ADD,
    OperatorVariant_BINARY_SUB,
    OperatorVariant_BINARY_LT,
    OperatorVariant_BINARY_GT,
    OperatorVariant_BINARY_LTE,
    OperatorVariant_BINARY_GTE,
    OperatorVariant_BINARY_EQ,
    OperatorVariant_BINARY_NEQ,
    OperatorVariant_BINARY_AND,
    OperatorVariant_BINARY_OR,
};

typedef struct {
    String op;
    OperatorType type;
    OperatorPriority priority;
    OperatorVariant variant;
} OperatorDecl;

SLICE_DECL(OperatorDecl)
SLICE_METHODS_DECL(compiler_opdeclslice, OperatorDecl)


#define AST_TYPES_LIST(X) \
    X(ASTNodeType_SYNTAX_ERROR, String, error) \
    X(ASTNodeType_LINT, int, l_int) \
    X(ASTNodeType_LNUM, double, l_num) \
    X(ASTNodeType_IDENT, ASTIdentData, ident) \
    X(ASTNodeType_IDENT_NESTED, ASTIdentData, nested_ident) \
    X(ASTNodeType_ENUM_DECL, ASTEnumDeclData, enum_decl) \
    X(ASTNodeType_OBJECT_DECL, ASTObjectDeclData, object_decl) \
    X(ASTNodeType_OBJECT_FIELD_DECL, ASTFieldDeclData, field_decl) \
    X(ASTNodeType_OBJECT_FIELD_SPEC, , ) \
    X(ASTNodeType_OBJECT_VARIANT_DECL, ASTObjectDeclData, variant_decl) \
    X(ASTNodeType_DMAN_FILE, , ) \

typedef enum ASTNodeType {
    AST_TYPES_LIST(ENUM_MEMBERS)
} ASTNodeType;

// AST node data for identifiers
typedef struct ASTIdentData {
    String name;
    LangSymbol *references_to;
} ASTIdentData;

typedef struct {
    String name;
    size_t n_members;
} ASTEnumDeclData;

typedef struct {
    String name;
    size_t n_fields;
    size_t n_variants;
} ASTObjectDeclData;

typedef struct {
    String name;
    String type_name;
} ASTFieldDeclData;

struct ASTNode {
    ASTNode *first_child;
    ASTNode *next_sibling;

    // Part of the document that corresponds to the node
    DocumentRange full_range;
    // Part of the document that is specific only to this node (e.g. the operator to a binop node)
    DocumentRange basis_range;

    // Additional data, as defined by .type
    union {
        AST_TYPES_LIST(UNION_FIELDS)
    } data;
    ASTNodeType type;
};

extern void compiler_astnode_print(ASTNode *node, size_t indent);


struct Compiler {
    // program source
    String source;
    // Diagnostic messages
    DiagnosticArray diagnostics;
    // allocator
    Arena *arena;
    ASTNode *result;
};

extern void compiler_init(Compiler *c, Arena *arena, String source);
extern void compiler_compile(Compiler *c, DmanEnv *env);
extern void compiler_print_diagnostics(Compiler c);
extern bool compiler_has_errors(Compiler c);

#endif
