#include <stdlib.h>
#include <stdio.h>
#include "util.h"
#include "lang.h"
#include "compiler.h"

int main() {
    Arena *arena = arena_new(65536); // TODO: separate arenas for parsing and for env
    arena_set_max_regions(arena, 1);

    FILE *input = fopen("./tests/test.dman", "r");
    StringBuilder text_builder = { 0 };
    strb_init(&text_builder, arena, 16384);
    if (strb_read_from_stream(&text_builder, input) != 0) {
        printf("Failed to read the file\n");
        arena_destroy(arena);
        return EXIT_FAILURE;
    }
    String program_text = strb_render(&text_builder);

    Compiler c;
    compiler_init(&c, arena, program_text);

    DmanEnv env;
    lang_env_init(&env, arena);

    compiler_compile(&c, &env);
    compiler_astnode_print(c.result, 0);

    compiler_print_diagnostics(c);
    if (compiler_has_errors(c)) {
        printf("Aborting due to previous errors\n");
        arena_destroy(arena);
        return EXIT_FAILURE;
    }

    lang_env_print(env);

    arena_destroy(arena);
    return EXIT_SUCCESS;
}
