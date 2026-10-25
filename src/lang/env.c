#include <stdlib.h>
#include <stdio.h>
#include "util.h"
#include "lang_int.h"

void lang_env_init(DmanEnv *env, Arena *arena) {
    env->arena = arena;
    env->data_types = arena_alloc(arena, sizeof(*env->data_types));
    env->symbols = arena_alloc(arena, sizeof(*env->symbols));

    _lang_typeregistry_init(env->data_types, arena);
    _lang_symtree_init(env->symbols, arena);

    _lang_env_add_prelude(env);
}

void lang_env_print(DmanEnv env) {
    _lang_typeregistry_print(env.data_types);
    _lang_symtree_print(env.symbols);
}

WellKnownTypes lang_env_get_wkt(DmanEnv env) {
    return env.data_types->wkt;
}
