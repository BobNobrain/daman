#include <stdio.h>
#include "lang_int.h"

void _lang_print_indent(size_t indent) {
    for (size_t i = 0; i < indent; i++) {
        printf(" ");
    }
}
