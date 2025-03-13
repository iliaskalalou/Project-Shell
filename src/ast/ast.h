#ifndef AST_H
#define AST_H

#include <unistd.h>

#include "token.h"

/*
enum token_type_ast
{
    AST_FUNCTION,
    AST_EOL,
    AST_EOF,
    AST_IF,
    AST_THEN,
    AST_ELIF,
    AST_ELSE,
    AST_FI,
    AST_SINGLE_QUOTE,
    AST_ERREUR,
};*/

/**
 This very simple AST structure should be sufficient for a simple AST.
 * It is however, NOT GOOD ENOUGH for more complicated projects, such as a
 * shell. Please read the project guide for some insights about other kinds of
 * ASTs.
 **/
struct ast
{
    enum token_type type; // The kind of node we're dealing with
    char *value; // If the node is a number, it stores its value
    struct ast **children; // The left branch if any, unary or binary
    size_t nb_children; // The right branch of the binary node
    size_t capacity;
};

// struct add_children(enum ast_type type, struct ast *tree, char *value);
/*
  \brief Allocates a new ast with the given type.
*/
struct ast *ast_new(enum token_type type);

/*
  \brief Recursively frees the given ast.
*/
void ast_free(struct ast *ast);

#endif /* !AST_H */
