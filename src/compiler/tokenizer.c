#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "util.h"
#include "compiler_int.h"

ARRAY_METHODS_IMPL(_compiler_tokenarray, Token)
SLICE_METHODS_IMPL(_compiler_tokenslice, Token)
SLICE_ARRAY_METHODS_IMPL(_compiler_tokenarray, Token)

void _compiler_tokenizer_init(Tokenizer *t, Compiler *c) {
    _compiler_tokenarray_init(&t->tokens, 128, c->arena);
    t->cursor = compiler_document_pos_zero();
    t->source = c->source;
}

void _compiler_tokenizer_push_token(Tokenizer *t, Token token) {
    // this glues together consecutive tokens of the same type
    // for newlines and invalid tokens
    if ((token.type == TokenType_INVALID || token.type == TokenType_NEWLINE) &&
        t->tokens.size > 0 &&
        t->tokens.content[t->tokens.size - 1].type == token.type
    ) {
        t->tokens.content[t->tokens.size - 1].range.end = token.range.end;
        return;
    }

    _compiler_tokenarray_append(&t->tokens, token);
}

typedef struct TokenScannerResult {
    size_t chars_scanned;
    bool cursor_updated;
    bool token_set;
    bool token_range_set;
} TokenScannerResult;

TokenScannerResult _compiler_result_ok(size_t chars_scanned) {
    return (TokenScannerResult) {
        .chars_scanned = chars_scanned,
        .cursor_updated = false,
        .token_set = true,
        .token_range_set = false,
    };
}
TokenScannerResult _compiler_result_fail() {
    return (TokenScannerResult) {
        .chars_scanned = 0,
        .cursor_updated = false,
        .token_set = false,
        .token_range_set = false,
    };
}

size_t _compiler_tokenizer_scan_ws(Tokenizer *t) {
    for (size_t i = 0; i < t->source.size; i++) {
        char next = t->source.content[i];

        switch (next) {
        case ' ':
        case '\t':
            break;

        default:
            return i;
        }
    }

    return t->source.size;
}

size_t _compiler_tokenizer_scan_line_comment(Tokenizer *t) {
    static const String LINE_COMMENT_START = STR_CONST("#");

    if (!str_starts_with(t->source, LINE_COMMENT_START)) {
        return 0;
    }

    for (size_t i = LINE_COMMENT_START.size; i < t->source.size; i++) {
        char next = t->source.content[i];

        switch (next) {
        case '\r':
        case '\n':
            return i;

        default: break;
        }
    }

    return t->source.size;
}

TokenScannerResult _compiler_tokenizer_scan_newline(Tokenizer *t, Token *into) {
    if (t->source.size == 0) { return _compiler_result_fail(); }

    size_t consumed = 0;
    char next = t->source.content[0];

    if (next == '\n') {
        consumed = 1;
    } else if (next == '\r') {
        if (t->source.size > 1 && t->source.content[1] == '\n') {
            consumed = 2;
        } else {
            consumed = 1;
        }
    }

    if (consumed > 0) {
        into->type = TokenType_NEWLINE;
        into->range = compiler_document_range(t->cursor, consumed);
        compiler_document_pos_line_break(&t->cursor);
        return (TokenScannerResult) {
            .chars_scanned = consumed,
            .cursor_updated = true,
            .token_set = true,
            .token_range_set = true,
        };
    }

    return _compiler_result_fail();
}

TokenScannerResult _compiler_tokenizer_scan_number(Tokenizer *t, Token *into) {
    bool point_met = false;
    size_t i = 0;

    for (; i < t->source.size; i++) {
        char next = t->source.content[i];

        if ('0' <= next && next <= '9') {
            continue;
        }
        if (next == '.' && !point_met) {
            point_met = true;
            continue;
        }

        break;
    }

    if (i == 0) {
        return _compiler_result_fail();
    }

    if (point_met && i == 1) {
        // a single point is not a valid number literal
        into->type = TokenType_DOT;
        return _compiler_result_ok(i);
    }

    if (point_met) {
        into->type = TokenType_FLOAT_LITERAL;
    } else {
        into->type = TokenType_INT_LITERAL;
    }

    return _compiler_result_ok(i);
}

TokenScannerResult _compiler_tokenizer_scan_ident(Tokenizer *t, Token *into) {
    size_t i = 0;

    for (; i < t->source.size; i++) {
        char next = t->source.content[i];

        if ('A' <= next && next <= 'Z') {
            continue;
        }
        if ('a' <= next && next <= 'z') {
            continue;
        }
        if ('0' <= next && next <= '9') {
            continue;
        }
        if (next == '_') {
            continue;
        }

        break;
    }

    if (i == 0) {
        return _compiler_result_fail();
    }

    into->type = TokenType_IDENT;
    return _compiler_result_ok(i);
}

TokenScannerResult _compiler_tokenizer_scan_op(Tokenizer *t, Token *into) {
    size_t i = 0;
    for (; i < t->source.size; i++) {
        char next = t->source.content[i];
        bool is_op_char = false;
        switch (next) {
        case '+': case '-': case '*': case '/': case '%':
        case '@': case '$': case ':': case '.': case '=':
        case '<': case '>': case '&': case '^': case '~':
            is_op_char = true;
        break;

        default: break;
        }

        if (!is_op_char) {
            break;
        }
    }

    if (i == 0) {
        return _compiler_result_fail();
    }

    into->type = TokenType_OPERATOR;
    return _compiler_result_ok(i);
}

TokenScannerResult _compiler_tokenizer_scan_paren(Tokenizer *t, Token *into) {
    if (t->source.size == 0) {
        return _compiler_result_fail();
    }

    char next = t->source.content[0];
    switch (next) {
    case '(':
        into->type = TokenType_PAREN_OPEN;
        break;
    case ')':
        into->type = TokenType_PAREN_CLOSE;
        break;
    case '{':
        into->type = TokenType_BRACE_OPEN;
        break;
    case '}':
        into->type = TokenType_BRACE_CLOSE;
        break;

    default:
        return _compiler_result_fail();
    }

    return _compiler_result_ok(1);
}

TokenScannerResult _compiler_tokenizer_scan_builtin_op(Tokenizer *t, Token *into) {
    if (t->source.size == 0) {
        return _compiler_result_fail();
    }

    char next = t->source.content[0];
    switch (next) {
    case '$':
        into->type = TokenType_DOLLAR;
        break;
    case '.':
        into->type = TokenType_DOT;
        break;
    case ',':
        into->type = TokenType_COMMA;
        break;

    default:
        return _compiler_result_fail();
    }

    return _compiler_result_ok(1);
}

#define NARROW_TOKENIZER_KEYWORD(CONST_NAME, C_STR) \
    else if (str_eq(token_content, STR_CONST(C_STR))) { \
        token->data.ident.keyword = CONST_NAME;         \
    }

void _compiler_tokenizer_parse_token(Token *token, String source, DiagnosticArray *errors) {
    String token_content = compiler_document_substring(source, token->range);

    switch (token->type) {
    case TokenType_IDENT:
        token->data.ident.name = token_content;
        token->data.ident.keyword = Keyword_NONE;

        if (false) {}
        KEYWORD_LIST(NARROW_TOKENIZER_KEYWORD)
    break;

    case TokenType_INT_LITERAL: {
        unsigned int parsed;
        size_t n_chars = str_parse_uint_dec(token_content, &parsed);

        if (n_chars != token_content.size) {
            compiler_diagnostic_array_append(errors, (Diagnostic) {
                .location = token->range,
                .message = STR_CONST("failed to parse an integer"),
                .severity = DiagnosticSeverity_ERROR,
                .source = DiagnosticSource_SYNTAX,
            });
            return;
        }

        token->data.l_int = (int) parsed;
    break;
    }

    case TokenType_FLOAT_LITERAL: {
        unsigned int whole, frac;
        size_t n_chars_whole = str_parse_uint_dec(token_content, &whole);
        String frac_str = str_substring(token_content, n_chars_whole + 1, token_content.size);
        size_t n_chars_frac = str_parse_uint_dec(frac_str, &frac);

        if (n_chars_whole + n_chars_frac + 1 != token_content.size) {
            compiler_diagnostic_array_append(errors, (Diagnostic) {
                .location = token->range,
                .message = STR_CONST("failed to parse a float"),
                .severity = DiagnosticSeverity_ERROR,
                .source = DiagnosticSource_SYNTAX,
            });
            return;
        }

        unsigned int frac_size = 1;
        for (size_t i = 0; i < n_chars_frac; i++) {
            frac_size *= 10;
        }
        token->data.l_float = (float) whole + ((float) frac) / ((float) frac_size);
    break;
    }

    case TokenType_OPERATOR:
        token->data.op = compiler_document_substring(source, token->range);
        if (str_eq(token->data.op, STR_CONST("$"))) {
            token->type = TokenType_DOLLAR;
        } else if (str_eq(token->data.op, STR_CONST("."))) {
            token->type = TokenType_DOT;
        }
    break;

    case TokenType_INVALID:
        compiler_diagnostic_array_append(errors, (Diagnostic) {
            .location = token->range,
            .message = STR_CONST("invalid token"),
            .severity = DiagnosticSeverity_ERROR,
            .source = DiagnosticSource_SYNTAX,
        });
    break;

    default: return;
    }
}

#undef NARROW_TOKENIZER_KEYWORD

void _compiler_tokenizer_consume_source(Tokenizer *t, size_t n) {
    t->source = str_substring(t->source, n, t->source.size);
}

void _compiler_tokenizer_run(Tokenizer *t, String source, DiagnosticArray *errors) {
    t->source = source;
    t->cursor = compiler_document_pos_zero();

    Token current = { .type = TokenType_INVALID };
    TokenScannerResult scan_result = { 0 };

    #define TRY_TOKEN_SCANNER(READER) \
    scan_result = READER(t, &current);                                                      \
    if (scan_result.chars_scanned > 0) {                                                    \
        _compiler_tokenizer_consume_source(t, scan_result.chars_scanned);                     \
        if (scan_result.token_set) {                                                        \
            if (!scan_result.token_range_set) {                                             \
                current.range = compiler_document_range(t->cursor, scan_result.chars_scanned);\
            }                                                                               \
            _compiler_tokenizer_push_token(t, current);                                        \
        }                                                                                   \
        if (!scan_result.cursor_updated) {                                                  \
            compiler_document_pos_track(&t->cursor, scan_result.chars_scanned);               \
        }                                                                                   \
        continue;                                                                           \
    }

    while (t->source.size > 0) {
        size_t ws_consumed = _compiler_tokenizer_scan_ws(t);
        if (ws_consumed > 0) {
            _compiler_tokenizer_consume_source(t, ws_consumed);
            compiler_document_pos_track(&t->cursor, ws_consumed);

            if (t->source.size == 0) { break; }
        }

        size_t comment_consumed = _compiler_tokenizer_scan_line_comment(t);
        if (comment_consumed > 0) {
            _compiler_tokenizer_consume_source(t, comment_consumed);
            compiler_document_pos_track(&t->cursor, comment_consumed);

            continue;
        }

        TRY_TOKEN_SCANNER(_compiler_tokenizer_scan_number)
        TRY_TOKEN_SCANNER(_compiler_tokenizer_scan_op)
        TRY_TOKEN_SCANNER(_compiler_tokenizer_scan_builtin_op)
        TRY_TOKEN_SCANNER(_compiler_tokenizer_scan_paren)
        TRY_TOKEN_SCANNER(_compiler_tokenizer_scan_ident)
        TRY_TOKEN_SCANNER(_compiler_tokenizer_scan_newline)

        // cannot consume the rest, must be invalid input
        current.type = TokenType_INVALID;
        current.range = compiler_document_range(t->cursor, 1);
        _compiler_tokenizer_push_token(t, current);
        _compiler_tokenizer_consume_source(t, 1);
        compiler_document_pos_track(&t->cursor, 1);
    }

    #undef TRY_TOKEN_SCANNER

    for (size_t i = 0; i < t->tokens.size; i++) {
        _compiler_tokenizer_parse_token(&t->tokens.content[i], source, errors);
    }
}

#define PRINT_TOKENIZER_KEYWORD_CASE(CONST_NAME, C_STR) \
    case CONST_NAME: printf("<kw:" C_STR "> "); break;

void _compiler_token_print(Token token) {
    switch (token.type) {
    case TokenType_IDENT:
        switch (token.data.ident.keyword) {
            case Keyword_NONE:
                printf("<ident:" STR_FMT "> ", STR_FMT_VAL(token.data.ident.name));
            break;

            KEYWORD_LIST(PRINT_TOKENIZER_KEYWORD_CASE)

            default: printf("<unknown kw '"STR_FMT"'> ", STR_FMT_VAL(token.data.ident.name)); break;
        }
    break;

    case TokenType_INT_LITERAL:   printf("<int:%d> ", token.data.l_int);                   break;
    case TokenType_FLOAT_LITERAL: printf("<float:%f> ", token.data.l_float);               break;
    case TokenType_OPERATOR:      printf("<op:" STR_FMT "> ", STR_FMT_VAL(token.data.op)); break;

    case TokenType_PAREN_OPEN:    printf("( ");      break;
    case TokenType_PAREN_CLOSE:   printf(") ");      break;
    case TokenType_BRACE_OPEN:    printf("{ ");      break;
    case TokenType_BRACE_CLOSE:   printf("} ");      break;
    case TokenType_NEWLINE:       printf("<\\n>\n"); break;
    case TokenType_DOLLAR:        printf("$ ");      break;
    case TokenType_DOT:           printf(". ");      break;
    case TokenType_COMMA:         printf(", ");      break;

    case TokenType_INVALID:       printf("<?> ");    break;
    default:                      printf("?");       break;
    }
}

void _compiler_tokenslice_print(TokenSlice tokens) {
    for (size_t i = 0; i < tokens.size; i++) {
        _compiler_token_print(tokens.content[i]);
    }

    printf("\n");
}

TokenSlice _compiler_run_tokenization(Compiler *c) {
    Tokenizer t = { 0 };
    _compiler_tokenizer_init(&t, c);
    _compiler_tokenizer_run(&t, c->source, &c->diagnostics);
    TokenSlice tokens = _compiler_tokenarray_seal(&t.tokens);
    return tokens;
}
