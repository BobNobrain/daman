#include <stdio.h>
#include "lang_int.h"


LangSymbolTreeItem* _lang_symtree_find_item(LangSymbolTreeItem *parent, String fqn);
void _lang_symtree_print_item(LangSymbolTreeItem *item, size_t indent);

void _lang_symtree_init(SymbolTree *tree, Arena *arena) {
    tree->arena = arena;
    tree->root = arena_alloc(arena, sizeof(*tree->root));

    *tree->root = (LangSymbolTreeItem) {
        .symbol = {
            .name = STR_EMPTY,
            .type = SymbolType_NONE,
            .value = { 0 },
        },
        .first_child = 0,
        .next_sibling = 0,
    };
}

LangSymbol* _lang_symtree_lookup(SymbolTree *tree, String fqn) {
    LangSymbolTreeItem *found = _lang_symtree_find_item(tree->root, fqn);
    if (found == 0) {
        return 0;
    }

    return &found->symbol;
}

LangSymbolTreeItem* _lang_symtree_insert(SymbolTree *tree, String fqn, LangSymbol symbol) {
    int last_dot = str_last_index_of(fqn, '.');
    LangSymbolTreeItem *parent = tree->root;
    if (last_dot != -1) {
        parent = _lang_symtree_find_item(tree->root, str_substring(fqn, 0, (size_t) last_dot));
    }

    LangSymbolTreeItem *new_item = _lang_symtree_alloc_item(tree->arena, symbol);

    if (last_dot == -1) {
        new_item->symbol.name = fqn;
    } else {
        new_item->symbol.name = str_substring(fqn, (size_t) last_dot + 1, fqn.size);
    }

    _lang_symtree_append_child(parent, new_item);
    return new_item;
}

LangSymbolTreeItem* _lang_symtree_find_item(LangSymbolTreeItem *parent, String fqn) {
    int first_dot = str_index_of(fqn, '.');
    bool is_last_part = first_dot == -1;
    String name_to_search = fqn;

    if (!is_last_part) {
        name_to_search = str_substring(fqn, 0, (size_t) first_dot);
    }

    // TODO: use a hashtable instead of linear search
    for (LangSymbolTreeItem *it = parent->first_child; it != 0; it = it->next_sibling) {
        if (!str_eq(it->symbol.name, name_to_search)) { continue; }

        if (is_last_part) {
            return it;
        }
        if (it->first_child == 0) {
            return 0;
        }

        return _lang_symtree_find_item(it, str_substring(fqn, (size_t) first_dot + 1, fqn.size));
    }

    return 0;
}

void _lang_symtree_append_child(LangSymbolTreeItem *parent, LangSymbolTreeItem *child) {
    // appending from the start, bc for now nobody cares about the order, and it's easier
    child->next_sibling = parent->first_child;
    parent->first_child = child;
}

LangSymbolTreeItem* _lang_symtree_alloc_item(Arena *arena, LangSymbol symbol) {
    LangSymbolTreeItem *new_item = arena_alloc(arena, sizeof(*new_item));
    *new_item = (LangSymbolTreeItem) {
        .first_child = 0,
        .next_sibling = 0,
        .symbol = symbol,
    };
    return new_item;
}

void _lang_symtree_print(SymbolTree *tree) {
    printf("SymbolTree: \n");
    _lang_symtree_print_item(tree->root, 0);
}

void _lang_symtree_print_item(LangSymbolTreeItem *item, size_t indent) {
    if (item == 0) {
        printf("<null>");
        return;
    }

    for (size_t i = 0; i < indent; i++) {
        printf("  ");
    }

    printf("- '"STR_FMT"': ", STR_FMT_VAL(item->symbol.name));

    switch (item->symbol.type) {
    case SymbolType_NONE:           printf("--");             break;
    case SymbolType_DATATYPE:       printf("datatype");       break;
    case SymbolType_ENCODING:       printf("encoding");       break;
    case SymbolType_ENUM_MEMBER:    printf("enum member");    break;
    case SymbolType_OBJECT_FIELD:   printf("object field");   break;
    case SymbolType_OBJECT_VARIANT: printf("object variant"); break;
    default:                        printf("??");             break;
    }

    printf("\n");

    for (LangSymbolTreeItem *it = item->first_child; it != 0; it = it->next_sibling) {
        _lang_symtree_print_item(it, indent + 1);
    }
}
