#include "compiler_int.h"

void compiler_init(Compiler *c, Arena *arena, String source) {
    c->arena = arena;
    c->source = source;
    c->result = 0;
    compiler_diagnostic_array_init(&c->diagnostics, 32, arena);
}

void compiler_compile(Compiler *c, DmanEnv *env) {
    TokenSlice tokens = _compiler_run_tokenization(c);
    _compiler_tokenslice_print(tokens);
    c->result = _compiler_run_parsing(c, tokens, _compiler_g_dman_file);

    _compiler_populate_env(c, env);
}

void compiler_print_diagnostics(Compiler c) {
    for (size_t i = 0; i < c.diagnostics.size; i++) {
        compiler_diagnostic_print(c.diagnostics.content[i], c.source, c.arena);
    }
}

bool compiler_has_errors(Compiler c) {
    for (size_t i = 0; i < c.diagnostics.size; i++) {
        if (c.diagnostics.content[i].severity == DiagnosticSeverity_ERROR) {
            return true;
        }
    }

    return false;
}

