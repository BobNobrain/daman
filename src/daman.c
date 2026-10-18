#include <stdlib.h>
#include <stdio.h>
#include "util.h"
#include "lang.h"

int main() {
    Arena *arena = arena_new(8192);
    arena_set_max_regions(arena, 4);

    DmanEnv env;
    lang_env_init(&env, arena);
    lang_env_print(env);

    arena_destroy(arena);
    return EXIT_SUCCESS;
}
