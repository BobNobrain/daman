#include "util.h"
#include "lang_int.h"

void lang_env_init(DmanEnv *env, Arena *arena) {
    env->arena = arena;
    env->data_types = arena_alloc(arena, sizeof(TypeRegistry));

    _lang_typeregistry_init(env->data_types, arena);
    _lang_typeregistry_add_builtins(env->data_types, arena);
}

void lang_env_print(DmanEnv env) {
    lang_typeregistry_print(env.data_types);
}
