#ifndef COMPILER_H
#define COMPILER_H

#include "util.h"

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
SLICE_METHODS_DECL(ct_opdeclslice, OperatorDecl)


#define AST_TYPES_LIST(X) \
    X(ASTNodeType_SYNTAX_ERROR, String, error) \
    X(ASTNodeType_LINT, int, lint) \
    X(ASTNodeType_LFLOAT, float, lfloat) \
    X(ASTNodeType_LBOOL, bool, lbool) \
    X(ASTNodeType_IDENT, ASTIdentData, ident) \
    X(ASTNodeType_ENUM_DECL, , ) \
    X(ASTNodeType_ENUM_MEMBER, , ) \
    X(ASTNodeType_OBJECT_DECL, , ) \
    X(ASTNodeType_OBJECT_FIELD_DECL, , ) \
    X(ASTNodeType_OBJECT_FIELD_SPECIFIER, , ) \
    X(ASTNodeType_OBJECT_VARIANT_DECL, , ) \
    X(ASTNodeType_ENCODING_DECL, , ) \
    X(ASTNodeType_ENCODING_BODY, , ) \
    X(ASTNodeType_INSTRUCTION_CONDITIONAL, , ) \
    X(ASTNodeType_INSTRUCTION_MATCH, , ) \
    X(ASTNodeType_EXPR_FNCALL, , ) \
    X(ASTNodeType_EXPR_BINOP, OperatorDecl, binop) \
    X(ASTNodeType_EXPR_UNOP, OperatorDecl, unop) \
    X(ASTNodeType_DMAN_FILE, , ) \

typedef enum ASTNodeType {
    AST_TYPES_LIST(ENUM_MEMBERS)
} ASTNodeType;

// AST node data for assignments (let x = ?)
typedef struct ASTAssignmentData {
    String identifier;
    Token *ident_token;
} ASTAssignmentData;

// AST node data for identifiers
typedef struct ASTIdentData {
    String name;
    // 0 for unbound, 1+ for closest lambda arguments
    size_t bound_index;
} ASTIdentData;

// AST node data for lambdas (\x y.?)
typedef struct ASTLambdaData {
    size_t n_args;
    ASTNode *body;
} ASTLambdaData;

// AST node data for lambda args
typedef struct ASTLambdaArgData {
    String name;
    String type;
} ASTLambdaArgData;

struct ASTNode {
    ASTNode *first_child;
    ASTNode *next_sibling;

    // What document range corresponds to this node
    DocumentRange range;
    // The defining token of this node (main keyword, or operator, or the single token that generates the node)
    Token *base;

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

#endif
