#include "ast.h"

#include <err.h>
#include <stdlib.h>

struct ast *ast_new(enum token_type type)
{
    struct ast *new = calloc(1, sizeof(struct ast));
    if (!new)
        return NULL;
    new->type = type;
    new->children = malloc(sizeof(struct ast) * 4);
    new->capacity = 4;
    new->nb_children = 0;
    new->value = NULL;
    return new;
}
/*
struct add_children(struct ast *tree, struct ast *tre)
{
    struct ast *new = malloc(sizeof(struct ast));
    new->type = type;
    new->children = malloc(sizeof(struct ast) * 4)
    new->capacity = 4;
    new->value = value;
    new->nb_children = 0;

    tree[nb_children] = new;
    tree->nb_children++;
    return tree;
}*/

void ast_free(struct ast *tree)
{
    if (tree == NULL)
        return;

    for (size_t i = 0; i < tree->nb_children; ++i)
    {
        ast_free(tree->children[i]);
    }
    free(tree->children);
    free(tree->value);
    free(tree);
}
