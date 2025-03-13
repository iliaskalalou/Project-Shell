#include "parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "lexer.h"
#include "token.h"

/*

enum token_type_ast
{
    AST_FUNCTION,
    AST_EOL,
    AST_IF,
    AST_THEN,
    AST_ELIF,
    AST_ELSE,
    AST_FI,
    AST_SINGLE_QUOTE,
    AST_ERREUR,
};

struct ast
{
    enum token_type_ast type; // The kind of node we're dealing with
    char* value; // If the node is a number, it stores its value
    struct ast children; // The left branch if any, unary or binary
    size_t nb_children; // The right branch of the binary node
};

enum token_type
{
    FUNCTION,
    EOL,
    IF,
    THEN,
    ELIF,
    ELSE,
    FI,
    SINGLE_QUOTE,
    ERREUR,
};
*/

//   GRAMMAIRE SHEL
/**
 *
 *      E : | if C then E D fi ; W
 *          | FUNC ; W
 *
 *      C : | E
 *
 *      D : | else ; E
 *          | elif C then E D
 *          | null
 *
 *      W : | E
 *          | null
 *
 */

static struct ast *parse_and_or(enum parser_status *status,
                                struct lexer *lexer);

static struct ast *parse_pipeline(enum parser_status *status,
                                  struct lexer *lexer);

static struct ast *parse_else_clause(enum parser_status *status,
                                     struct lexer *lexer);

static struct ast *parse_rule_if(enum parser_status *status,
                                 struct lexer *lexer);

static struct ast *parse_command(enum parser_status *status,
                                 struct lexer *lexer);

static struct ast *parse_simple_command(enum parser_status *status,
                                        struct lexer *lexer);

/*static struct ast *parse_element(enum parser_status *status,
  struct lexer *lexer);*/

/*static int is_a_var(enum parser_status *status, struct token *tok,
                    struct lexer *lexer)
{
    if (tok->type != FUNCTION)
    {
        return 0;
    }
    size_t tmp = lexer->pos;
    // printf("%ld\n", tmp);
    if (tmp == 0)
    {
        lexer->arg2 = 0;
    }
    lexer_pop(status, lexer);
    tok = lexer_peek(status, lexer);
    if (tok->type != EQUAL && tok->type != VARIABLE_SUPP)
    {
        //	    printf("is var %s\n", tok->value);
        token_free(tok);
        lexer->pos = tmp;
        return 0;
    }

    if (tok->type == VARIABLE_SUPP)
    {
        token_free(tok);
        lexer_pop(status, lexer);
        tok = lexer_peek(status, lexer);
        while (tok->type == EOL)
        {
            //            lexer->input[lexer->pos] = ' ';
            token_free(tok);
            lexer_pop(status, lexer);
            lexer->input[lexer->pos] = ' ';
            tok = lexer_peek(status, lexer);
        }
        token_free(tok);

        return 2;
    }

    token_free(tok);
    lexer_pop(status, lexer);

    tok = lexer_peek(status, lexer);
    if (tok->type != FUNCTION && tok->type != VARIABLE
        && tok->type != VARIABLE_LEFT && tok->type != VARIABLE_RIGHT)
    {
        token_free(tok);
        lexer->pos = tmp;
        *status = PARSER_UNEXPECTED_TOKEN;

        return 0;
    }
    token_free(tok);

    lexer_pop(status, lexer);
    tok = lexer_peek(status, lexer);
    if (tok->type != EOL && tok->type != EOFF)
    {
        token_free(tok);
        return 0;
    }
    while (tok->type == EOL)
    {
        lexer_pop(status, lexer);
        lexer->input[lexer->pos - 1] = ' ';
        token_free(tok);
        // lexer_pop(status,lexer);
        tok = lexer_peek(status, lexer);
    }
    token_free(tok);
    lexer->pos = tmp;
    return 1;
    }*/

static size_t teste(enum parser_status *status, struct lexer *lexer)
{
    size_t tmp = lexer->pos;
    // size_t res = 0;

    struct token *tok = lexer_peek(status, lexer);

    while (tok != NULL && tok->type != EOFF)
    {
        //         printf("%u\n", tok->type);
        token_free(tok);
        tok = lexer_peek(status, lexer);
        // printf("tooooooo\n");
        if (tok != NULL && tok->type == EOL && strcmp(tok->value, "n") == 0)
        {
            //            printf("tooooooo\n");
            lexer_pop(status, lexer);
            token_free(tok);
            tok = lexer_peek(status, lexer);
            if (tok != NULL && tok->type == EOL && strcmp(tok->value, "v") == 0)
            {
                //              printf("tttttttt\n");
                token_free(tok);
                return 1;
            }
        }
        // printf("inter\n");
        lexer_pop(status, lexer);
        //  printf("fin2\n");
    }

    // printf("fin\n");

    token_free(tok);
    lexer->pos = tmp;

    return 0;
}

static size_t teste_v(enum parser_status *status, struct lexer *lexer)
{
    size_t tmp = lexer->pos;
    // size_t res = 0;

    struct token *tok = lexer_peek(status, lexer);

    while (tok != NULL && tok->type != EOFF)
    {
        //         printf("%u\n", tok->type);
        token_free(tok);
        tok = lexer_peek(status, lexer);
        // printf("tooooooo\n");
        if (tok != NULL && tok->type == EOL && strcmp(tok->value, "v") == 0)
        {
            //            printf("tooooooo\n");
            lexer_pop(status, lexer);
            token_free(tok);
            tok = lexer_peek(status, lexer);
            if (tok != NULL && tok->type == EOL && strcmp(tok->value, "v") == 0)
            {
                //              printf("tttttttt\n");
                token_free(tok);
                return 1;
            }
        }
        // printf("inter\n");
        lexer_pop(status, lexer);
        //  printf("fin2\n");
    }

    // printf("fin\n");

    token_free(tok);
    lexer->pos = tmp;

    return 0;
}

void supp_n_start(enum parser_status *status, struct lexer *lexer)
{
    struct token *tok = lexer_peek(status, lexer);

    while (tok->type == EOL && tok->value[0] == 'n')
    {
        token_free(tok);
        lexer_pop(status, lexer);

        tok = lexer_peek(status, lexer);
    }
    token_free(tok);
}

struct ast *parse(enum parser_status *status, struct lexer *lexer)
{
    *status = PARSER_OK;
    if (teste(status, lexer) == 1 || teste_v(status, lexer) == 1)
    {
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }
    lexer->arg2 = 0;
    supp_n_start(status, lexer);

    struct token *tok = lexer_peek(status, lexer);
    if (tok->type == EOFF)
    {
        token_free(tok);
        return NULL;
    }
    if (tok->type == EOL && tok->value[0] == 'v')
    {
        *status = PARSER_UNEXPECTED_TOKEN;
        token_free(tok);
        return NULL;
    }
    struct ast *tree = parse_list(status, lexer);
    if (*status != PARSER_OK || tree == NULL)
    {
        token_free(tok);
        return NULL;
    }
    token_free(tok);

    struct token *tok2 = lexer_peek(status, lexer);
    if (tok2->type != EOFF && tok2->type != EOL)
    {
        token_free(tok2);
        // fprintf(stderr, "erreur\n");
        *status = PARSER_UNEXPECTED_TOKEN;
        ast_free(tree);
        return NULL;
    }

    token_free(tok2);

    return tree;
}

void ast_add_child(struct ast *tree, struct ast *child)
{
    if (tree->nb_children == tree->capacity)
    {
        size_t c = tree->capacity * 2;
        struct ast **tmp = realloc(tree->children, c * sizeof(struct ast *));
        tree->children = tmp;
        tree->capacity = c;
    }
    tree->children[tree->nb_children] = child;
    tree->nb_children++;
}

struct ast *parse_list(enum parser_status *status, struct lexer *lexer)
{
    struct ast *node = parse_and_or(status, lexer);
    if (*status != PARSER_OK || node == NULL)
    {
        ast_free(node);
        return NULL;
    }

    struct ast *tree =
        ast_new(AST_LIST); // pour moi le AST_LIST c'est un peux comme
    // un noeud racine quand j'en est besoin (un ;)
    ast_add_child(tree, node);

    while (1)
    {
        struct token *tok = lexer_peek(status, lexer);
        if (tok->type == EOL)
        {
            lexer_pop(status, lexer);
            token_free(tok);

            struct token *tok2 = lexer_peek(status, lexer);
            if (tok2->type != EOFF && tok2->type != EOL)
            {
                struct ast *cmd = parse_and_or(status, lexer);
                if (*status != PARSER_OK || cmd == NULL)
                {
                    token_free(tok2);
                    ast_free(tree);
                    return NULL;
                }
                ast_add_child(tree, cmd);
            }

            token_free(tok2);
        }
        else
        {
            token_free(tok);
            break;
        }
    }

    return tree;
}

struct ast *parse_compound_list(enum parser_status *status, struct lexer *lexer)
{
    struct token *tok = lexer_peek(status, lexer);
    while (tok->type == EOL)
    {
        lexer_pop(status, lexer);
        token_free(tok);
        tok = lexer_peek(status, lexer);
    }
    token_free(tok);

    struct ast *res = ast_new(AST_LIST);
    struct ast *child = parse_and_or(status, lexer);
    if (child == NULL)
    {
        *status = PARSER_OK;
        ast_free(res);
        return NULL;
    }
    ast_add_child(res, child);
    child = parse_compound_list(status, lexer);
    if (!child)
    {
        return res;
    }
    ast_add_child(res, child);

    tok = lexer_peek(status, lexer);
    while (tok->type == EOL)
    {
        lexer_pop(status, lexer);
        //  token_free(tok);
        tok = lexer_peek(status, lexer);
    }
    token_free(tok);

    return res;
}

static struct ast *parse_and_or_part1(enum parser_status *status,
                                      struct lexer *lexer, struct ast *tree)
{
    struct ast *res = tree;
    while (1)
    {
        struct token *tok2 = lexer_peek(status, lexer);
        if (tok2->type != OR && tok2->type != AND)
        {
            token_free(tok2);
            break;
        }

        struct ast *tree2 = ast_new(tok2->type);

        token_free(tok2);
        lexer_pop(status, lexer);

        ast_add_child(tree2, tree);
        tree = tree2;
        res = tree2;

        struct ast *left2 = parse_pipeline(status, lexer);

        if (*status != PARSER_OK || left2 == NULL)
        {
            ast_free(res);
            if (left2)
            {
                ast_free(left2);
            }
            return NULL;
        }

        ast_add_child(tree2, left2);
    }
    return res;
}

static struct ast *parse_and_or(enum parser_status *status, struct lexer *lexer)
{
    struct ast *left = parse_pipeline(status, lexer);

    if (*status != PARSER_OK || left == NULL)
    {
        if (left)
        {
            ast_free(left);
        }
        return NULL;
    }

    struct token *tok = lexer_peek(status, lexer);
    if (tok->type != AND && tok->type != OR)
    {
        token_free(tok);
        return left;
    }
    token_free(tok);

    struct token *tok2 = lexer_peek(status, lexer);
    struct ast *tree = ast_new(tok2->type);

    token_free(tok2);
    lexer_pop(status, lexer);

    ast_add_child(tree, left);

    struct token *m = lexer_peek(status, lexer);
    while (m->type == EOL && m->value[0] == 'n')
    {
        token_free(m);
        lexer_pop(status, lexer);
        m = lexer_peek(status, lexer);
    }
    token_free(m);

    struct ast *res = tree;

    struct ast *left2 = parse_pipeline(status, lexer);

    if (*status != PARSER_OK || left2 == NULL)
    {
        ast_free(res);
        if (left2)
        {
            ast_free(left2);
        }
        return NULL;
    }

    ast_add_child(tree, left2);

    return parse_and_or_part1(status, lexer, tree);
}

struct parse_pipeline_ctx
{
    enum parser_status *status;
    struct lexer *lexer;
    struct ast *left;
    size_t p;
    struct ast *res2;
    struct ast *res;
};

static struct ast *parse_pipeline_partA(struct parse_pipeline_ctx *ctx)
{
    struct token *to;
    to = lexer_peek(ctx->status, ctx->lexer);

    if (to->type == NEGATION)
    {
        ctx->p = 1;
        token_free(to);
        lexer_pop(ctx->status, ctx->lexer);

        ctx->left = ast_new(NEGATION);
        ast_add_child(ctx->left, parse_pipeline(ctx->status, ctx->lexer));
        return ctx->left; // On renvoie direct un AST
    }
    else
    {
        token_free(to);
    }

    return (struct ast *)-1;
}

static struct ast *parse_pipeline_partB(struct parse_pipeline_ctx *ctx)
{
    if (ctx->left == NULL)
    {
        ctx->left = parse_command(ctx->status, ctx->lexer);
    }
    else
    {
        ast_add_child(ctx->left, parse_command(ctx->status, ctx->lexer));
        ctx->res2 = ctx->left;
        ctx->left = ctx->left->children[0];
    }

    if (*ctx->status != PARSER_OK || ctx->left == NULL)
    {
        if (ctx->left)
        {
            ast_free(ctx->left);
        }
        return NULL;
    }

    return (struct ast *)-1;
}

static struct ast *parse_pipeline_partC(struct parse_pipeline_ctx *ctx)
{
    struct token *tok;
    tok = lexer_peek(ctx->status, ctx->lexer);
    if (tok->type != PIPE)
    {
        token_free(tok);
        if (ctx->p == 1)
        {
            return ctx->res2;
        }
        return ctx->left;
    }

    token_free(tok);
    return (struct ast *)-1;
}

static struct ast *parse_pipeline_partD(struct parse_pipeline_ctx *ctx)
{
    struct token *tok;
    tok = lexer_peek(ctx->status, ctx->lexer);

    struct ast *tree;
    tree = ast_new(PIPE);
    tree->value = malloc(100000);
    strcpy(tree->value, tok->value);

    token_free(tok);
    lexer_pop(ctx->status, ctx->lexer);

    struct token *m;
    m = lexer_peek(ctx->status, ctx->lexer);
    while (m->type == EOL && m->value[0] == 'n')
    {
        token_free(m);
        lexer_pop(ctx->status, ctx->lexer);
        m = lexer_peek(ctx->status, ctx->lexer);
    }
    token_free(m);

    ast_add_child(tree, ctx->left);

    ctx->res = tree;
    struct ast *left2;
    left2 = parse_command(ctx->status, ctx->lexer);

    if (*ctx->status != PARSER_OK || left2 == NULL)
    {
        ast_free(tree);
        if (left2)
        {
            ast_free(left2);
        }
        return NULL;
    }
    ast_add_child(tree, left2);

    return tree;
}

static struct ast *parse_pipeline_partE(struct parse_pipeline_ctx *ctx)
{
    while (1)
    {
        struct token *tok;
        tok = lexer_peek(ctx->status, ctx->lexer);
        if (tok->type != PIPE)
        {
            token_free(tok);
            break;
        }

        struct ast *tree2;
        tree2 = ast_new(PIPE);
        tree2->value = malloc(100000);
        strcpy(tree2->value, tok->value);
        token_free(tok);
        lexer_pop(ctx->status, ctx->lexer);

        ast_add_child(tree2, ctx->res);
        ctx->res = tree2;

        struct ast *left2;
        left2 = parse_command(ctx->status, ctx->lexer);
        if (*ctx->status != PARSER_OK || left2 == NULL)
        {
            ast_free(ctx->res);
            if (left2)
            {
                ast_free(left2);
            }
            return NULL;
        }

        ast_add_child(tree2, left2);
    }

    if (ctx->p == 1)
    {
        return ctx->res2;
    }
    return ctx->res;
}

static struct ast *parse_pipeline(enum parser_status *status,
                                  struct lexer *lexer)
{
    struct parse_pipeline_ctx ctx;
    ctx.status = status;
    ctx.lexer = lexer;
    ctx.left = NULL;
    ctx.p = 0;
    ctx.res2 = NULL;
    ctx.res = NULL;

    struct ast *retA;
    retA = parse_pipeline_partA(&ctx);
    if (retA != (struct ast *)-1)
    {
        return retA;
    }

    struct ast *retB;
    retB = parse_pipeline_partB(&ctx);
    if (retB != (struct ast *)-1)
    {
        return retB;
    }

    struct ast *retC;
    retC = parse_pipeline_partC(&ctx);
    if (retC != (struct ast *)-1)
    {
        return retC;
    }

    struct ast *retD;
    retD = parse_pipeline_partD(&ctx);
    if (retD == NULL)
    {
        return NULL;
    }

    ctx.res = retD;

    struct ast *retE;
    retE = parse_pipeline_partE(&ctx);
    return retE;
}

/*
static struct ast *parse_pipeline(enum parser_status *status,
                                  struct lexer *lexer)
{
    struct ast *left = NULL;

    size_t p = 0;
    struct ast *res2 = left;

    struct token *to = lexer_peek(status, lexer);

    if (to->type == NEGATION)
    {
        p = 1;
        token_free(to);
        lexer_pop(status, lexer);

        left = ast_new(NEGATION);
        ast_add_child(left, parse_pipeline(status, lexer));
        return left;
    }
    else
    {
        token_free(to);
    }

    if (left == NULL)
    {
        left = parse_command(status, lexer);
    }
    else
    {
        ast_add_child(left, parse_command(status, lexer));
        res2 = left;
        left = left->children[0];
    }

    if (*status != PARSER_OK || left == NULL)
    {
        if (left)
        {
            ast_free(left);
        }
        return NULL;
    }

    struct token *tok = lexer_peek(status, lexer);
    if (tok->type != PIPE)
    {
        token_free(tok);
        if (p == 1)
        {
            return res2;
        }
        return left;
    }

    struct ast *tree = ast_new(PIPE);
    tree->value = malloc(100000);
    strcpy(tree->value, tok->value);

    token_free(tok);
    lexer_pop(status, lexer);

    struct token *m = lexer_peek(status, lexer);
    while (m->type == EOL && m->value[0] == 'n')
    {
        token_free(m);
        lexer_pop(status, lexer);
        m = lexer_peek(status, lexer);
    }

    token_free(m);

    ast_add_child(tree, left);

    struct ast *res = tree;

    struct ast *left2 = parse_command(status, lexer);

    if (*status != PARSER_OK || left2 == NULL)
    {
        if (left2)
        {
            ast_free(left2);
        }
        return NULL;
    }

    ast_add_child(tree, left2);

    while (1)
    {
        struct token *tok = lexer_peek(status, lexer);
        if (tok->type != PIPE)
        {
            token_free(tok);
            break;
        }

        struct ast *tree2 = ast_new(PIPE);
        tree2->value = malloc(100000);
        strcpy(tree2->value, tok->value);
        token_free(tok);
        lexer_pop(status, lexer);

        ast_add_child(tree, tree2);

        tree = tree2;

        struct ast *left2 = parse_command(status, lexer);

        if (*status != PARSER_OK || left2 == NULL)
        {
            if (left2)
            {
                ast_free(left2);
            }
            return NULL;
        }

        ast_add_child(tree2, left2);
    }

    if (p == 1)
    {
        return res2;
    }

    return res;
}
*/

static struct ast *parse_else_clause_part2(enum parser_status *status,
                                           struct lexer *lexer,
                                           struct ast *tree)
{
    struct token *tok = lexer_peek(status, lexer);
    if (tok->type != THEN)
    {
        ast_free(tree);
        // ast_free(cond);
        token_free(tok);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }
    lexer_pop(status, lexer);
    token_free(tok);

    struct ast *cl = parse_compound_list(status, lexer);
    if (*status != PARSER_OK || !cl)
    {
        if (cl)
        {
            ast_free(cl);
        }
        ast_free(tree);
        // ast_free(cond);
        return NULL;
    }

    ast_add_child(tree, cl);

    // else clause

    struct ast *elif = parse_else_clause(status, lexer);
    if (*status != PARSER_OK)
    {
        ast_free(elif);
        ast_free(tree);
        return NULL;
    }

    if (elif)
    {
        ast_add_child(tree, elif);
    }

    return tree;
}

static struct ast *parse_else_clause_part1(enum parser_status *status,
                                           struct lexer *lexer)
{
    struct token *tok = lexer_peek(status, lexer);
    if (tok->type == ELSE)
    {
        lexer_pop(status, lexer);
        token_free(tok);

        struct ast *tree = parse_compound_list(status, lexer);
        if (*status != PARSER_OK)
        {
            ast_free(tree);
            return NULL;
        }

        return tree;
    }
    else if (tok->type == ELIF)
    {
        lexer_pop(status, lexer);
        token_free(tok);

        struct ast *tree = ast_new(ELIF);
        // cond

        struct ast *cond = parse_compound_list(status, lexer);
        if (*status != PARSER_OK || !cond)
        {
            if (cond)
            {
                ast_free(cond);
            }
            ast_free(tree);
            return NULL;
        }

        ast_add_child(tree, cond);

        return parse_else_clause_part2(status, lexer, tree);
    }
    else
    {
        token_free(tok);
        return NULL;
    }
}

static struct ast *parse_else_clause(enum parser_status *status,
                                     struct lexer *lexer)
{
    return parse_else_clause_part1(status, lexer);
}

static struct ast *parse_rule_if_part3(enum parser_status *status,
                                       struct lexer *lexer, struct ast *tree)
{
    // elif

    struct ast *elif = parse_else_clause(status, lexer);
    if (*status != PARSER_OK)
    {
        ast_free(tree);
        // ast_free(cond);
        ast_free(elif);
        return NULL;
    }

    if (elif != NULL)
    {
        ast_add_child(tree, elif);
    }

    struct token *tok = lexer_peek(status, lexer);
    if (tok->type != FI)
    {
        token_free(tok);
        ast_free(tree);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }
    lexer_pop(status, lexer);
    token_free(tok);
    tok = lexer_peek(status, lexer);
    if (tok->type == REDIRECTION)
    {
        lexer_pop(status, lexer);
        struct token *tok2 = lexer_peek(status, lexer);
        if (tok2->type != FUNCTION)
        {
            token_free(tok);
            token_free(tok2);
            ast_free(tree);
            *status = PARSER_UNEXPECTED_TOKEN;
            return NULL;
        }
        struct ast *new = ast_new(FILEM);
        new->value = malloc(100000);
        strcpy(new->value, tok2->value);
        struct ast *new2 = ast_new(REDIRECTION);
        new2->value = malloc(100000);
        strcpy(new2->value, tok->value);
        ast_add_child(new, new2);
        ast_add_child(new, tree);

        lexer_pop(status, lexer);
        token_free(tok);
        token_free(tok2);
        return new;
    }
    token_free(tok);
    return tree;
}

static struct ast *parse_rule_if_part2(enum parser_status *status,
                                       struct lexer *lexer, struct ast *tree)
{
    // then
    struct token *tok = lexer_peek(status, lexer);
    if (tok->type != THEN)
    {
        // ast_free(cond);
        ast_free(tree);
        token_free(tok);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }
    lexer_pop(status, lexer);
    token_free(tok);

    struct ast *then = parse_compound_list(status, lexer);
    if (*status != PARSER_OK || !then)
    {
        if (then)
        {
            ast_free(then);
        }
        *status = PARSER_UNEXPECTED_TOKEN;
        ast_free(tree);
        // ast_free(cond);
        return NULL;
    }

    ast_add_child(tree, then);

    return parse_rule_if_part3(status, lexer, tree);
}

static struct ast *parse_rule_if_part1(enum parser_status *status,
                                       struct lexer *lexer)
{
    struct token *tok = lexer_peek(status, lexer);
    if (tok->type != IF)
    {
        token_free(tok);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }
    struct ast *tree = ast_new(IF);

    lexer_pop(status, lexer);
    token_free(tok);

    tok = lexer_peek(status, lexer);
    if (tok->type == EOL && tok->value[0] == 'v')
    {
        ast_free(tree);
        *status = PARSER_UNEXPECTED_TOKEN;
        token_free(tok);
        return NULL;
    }
    token_free(tok);

    struct ast *cond = parse_compound_list(status, lexer);
    if (*status != PARSER_OK || !cond)
    {
        ast_free(tree);
        if (cond)
        {
            ast_free(cond);
        }
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }

    ast_add_child(tree, cond);

    return parse_rule_if_part2(status, lexer, tree);
}

static struct ast *parse_rule_if(enum parser_status *status,
                                 struct lexer *lexer)
{
    return parse_rule_if_part1(status, lexer);
}

/*
static struct ast *parse_rule_if(enum parser_status *status,
                                 struct lexer *lexer)
{
    struct token *tok = lexer_peek(lexer);
    if (tok->type != IF)
    {
        token_free(tok);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }
    struct ast *tree = ast_new(IF);

    lexer_pop(lexer);
    token_free(tok);

    // condution

    struct ast *cond = parse_compound_list(status, lexer);
    if (*status != PARSER_OK || !cond)
    {
        ast_free(tree);
        if (cond)
        {
            ast_free(cond);
        }
        return NULL;
    }

    ast_add_child(tree, cond);

    // then

    tok = lexer_peek(lexer);
    if (tok->type != THEN)
    {
        // ast_free(cond);
        ast_free(tree);
        free(tok);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }
    lexer_pop(lexer);
    token_free(tok);

    struct ast *then = parse_compound_list(status, lexer);
    if (*status != PARSER_OK || !then)
    {
        if (then)
        {
            ast_free(then);
        }
        ast_free(tree);
        // ast_free(cond);
        return NULL;
    }

    ast_add_child(tree, then);

    // elif

    struct ast *elif = parse_else_clause(status, lexer);
    if (*status != PARSER_OK)
    {
        ast_free(tree);
        // ast_free(cond);
        ast_free(elif);
        return NULL;
    }

    if (elif != NULL)
    {
        ast_add_child(tree, elif);
    }

    tok = lexer_peek(lexer);
    if (tok->type != FI)
    {
        // ast_free(cond);
        // ast_free(then);
        // ast_free(elif);
        token_free(tok);
        ast_free(tree);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }
    lexer_pop(lexer);
    token_free(tok);

    return tree;
}*/

static struct ast *handle_redirection_while(enum parser_status *status,
                                            struct lexer *lexer,
                                            struct ast *tree, struct token *tok)
{
    lexer_pop(status, lexer);

    struct token *tok2;
    tok2 = lexer_peek(status, lexer);

    if (tok2->type != FUNCTION)
    {
        token_free(tok);
        token_free(tok2);
        ast_free(tree);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }

    struct ast *new_ast;
    new_ast = ast_new(FILEM);
    new_ast->value = malloc(100000);
    strcpy(new_ast->value, tok2->value);

    struct ast *new2;
    new2 = ast_new(REDIRECTION);
    new2->value = malloc(100000);
    strcpy(new2->value, tok->value);

    ast_add_child(new_ast, new2);
    ast_add_child(new_ast, tree);

    lexer_pop(status, lexer);
    token_free(tok);
    token_free(tok2);

    return new_ast;
}

static struct ast *parse_rule_while_part2(enum parser_status *status,
                                          struct lexer *lexer, struct ast *tree)
{
    struct token *tok;
    tok = lexer_peek(status, lexer);

    if (tok->type != DO)
    {
        ast_free(tree);
        token_free(tok);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }

    lexer_pop(status, lexer);
    token_free(tok);

    struct ast *then;
    then = parse_compound_list(status, lexer);

    if (*status != PARSER_OK || !then)
    {
        if (then)
        {
            ast_free(then);
        }
        *status = PARSER_UNEXPECTED_TOKEN;
        ast_free(tree);
        return NULL;
    }

    ast_add_child(tree, then);

    struct token *tok2;
    tok2 = lexer_peek(status, lexer);

    if (tok2->type != DONE)
    {
        ast_free(tree);
        token_free(tok2);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }

    lexer_pop(status, lexer);
    token_free(tok2);

    tok = lexer_peek(status, lexer);

    if (tok->type == REDIRECTION)
    {
        struct ast *res;
        res = handle_redirection_while(status, lexer, tree, tok);
        return res;
    }

    token_free(tok);
    return tree;
}

/*
static struct ast *parse_rule_while_part2(enum parser_status *status,
                                          struct lexer *lexer, struct ast *tree)
{
    // then

    struct token *tok = lexer_peek(status, lexer);
    if (tok->type != DO)
    {
        // ast_free(cond);
        ast_free(tree);
        token_free(tok);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }
    lexer_pop(status, lexer);
    token_free(tok);

    struct ast *then = parse_compound_list(status, lexer);
    if (*status != PARSER_OK || !then)
    {
        if (then)
        {
            ast_free(then);
        }
        *status = PARSER_UNEXPECTED_TOKEN;
        ast_free(tree);
        // ast_free(cond);
        return NULL;
    }

    ast_add_child(tree, then);

    struct token *tok2 = lexer_peek(status, lexer);
    if (tok2->type != DONE)
    {
        // ast_free(cond);
        ast_free(tree);
        token_free(tok2);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }
    lexer_pop(status, lexer);
    token_free(tok2);
    tok = lexer_peek(status, lexer);
    if (tok->type == REDIRECTION)
    {
        lexer_pop(status, lexer);
        struct token *tok2 = lexer_peek(status, lexer);
        if (tok2->type != FUNCTION)
        {
            token_free(tok);
            token_free(tok2);
            ast_free(tree);
            *status = PARSER_UNEXPECTED_TOKEN;
            return NULL;
        }
        struct ast *new = ast_new(FILEM);
        new->value = malloc(100000);
        strcpy(new->value, tok2->value);
        struct ast *new2 = ast_new(REDIRECTION);
        new2->value = malloc(100000);
        strcpy(new2->value, tok->value);
        ast_add_child(new, new2);
        ast_add_child(new, tree);

        lexer_pop(status, lexer);
        token_free(tok);
        token_free(tok2);
        return new;
    }
    token_free(tok);
    return tree;
}
*/

static struct ast *parse_rule_while_part1(enum parser_status *status,
                                          struct lexer *lexer)
{
    struct token *tok = lexer_peek(status, lexer);
    if (tok->type != WHILE)
    {
        token_free(tok);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }
    struct ast *tree = ast_new(WHILE);

    lexer_pop(status, lexer);
    token_free(tok);

    // condution

    // verifier que c'est bien une FUNUC apres
    /*struct token *tok2 = lexer_peek(status, lexer);
    if (tok2->type != FUNCTION && tok2->type != VARIABLE
        && tok2->type != NEGATION && tok2->type != EXEC
        && tok2->type != FUNCTION_RIGHT && tok2->type != FUNCTION_LEFT
        && tok2->type != VARIABLE_RIGHT && tok2->type != VARIABLE_LEFT
    && tok2->type != FUNCTION_MIDDLE)
    {
        ast_free(tree);
        token_free(tok2);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }
    token_free(tok2);*/

    struct ast *cond = parse_compound_list(status, lexer);
    if (*status != PARSER_OK || !cond)
    {
        ast_free(tree);
        if (cond)
        {
            ast_free(cond);
        }
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }

    ast_add_child(tree, cond);

    return parse_rule_while_part2(status, lexer, tree);
}

static struct ast *parse_rule_while(enum parser_status *status,
                                    struct lexer *lexer)
{
    return parse_rule_while_part1(status, lexer);
}

static struct ast *handle_redirection_until(enum parser_status *status,
                                            struct lexer *lexer,
                                            struct ast *tree, struct token *tok)
{
    lexer_pop(status, lexer);
    struct token *tok2;
    tok2 = lexer_peek(status, lexer);

    if (tok2->type != FUNCTION)
    {
        token_free(tok);
        token_free(tok2);
        ast_free(tree);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }

    struct ast *new_ast;
    new_ast = ast_new(FILEM);
    new_ast->value = malloc(100000);
    strcpy(new_ast->value, tok2->value);

    struct ast *new2;
    new2 = ast_new(REDIRECTION);
    new2->value = malloc(100000);
    strcpy(new2->value, tok->value);

    ast_add_child(new_ast, new2);
    ast_add_child(new_ast, tree);

    lexer_pop(status, lexer);
    token_free(tok);
    token_free(tok2);

    return new_ast;
}

static struct ast *parse_rule_until_part2(enum parser_status *status,
                                          struct lexer *lexer, struct ast *tree)
{
    // then
    struct token *tok;
    tok = lexer_peek(status, lexer);
    if (tok->type != DO)
    {
        ast_free(tree);
        token_free(tok);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }
    lexer_pop(status, lexer);
    token_free(tok);

    struct ast *then;
    then = parse_compound_list(status, lexer);
    if (*status != PARSER_OK || !then)
    {
        if (then)
        {
            ast_free(then);
        }
        *status = PARSER_UNEXPECTED_TOKEN;
        ast_free(tree);
        return NULL;
    }
    ast_add_child(tree, then);

    struct token *tok2;
    tok2 = lexer_peek(status, lexer);
    if (tok2->type != DONE)
    {
        ast_free(tree);
        token_free(tok2);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }
    lexer_pop(status, lexer);
    token_free(tok2);

    tok = lexer_peek(status, lexer);
    if (tok->type == REDIRECTION)
    {
        struct ast *result;
        result = handle_redirection_until(status, lexer, tree, tok);
        return result;
    }

    token_free(tok);
    return tree;
}

/*
static struct ast *parse_rule_until_part2(enum parser_status *status,
                                          struct lexer *lexer, struct ast *tree)
{
    // then

    struct token *tok = lexer_peek(status, lexer);
    if (tok->type != DO)
    {
        // ast_free(cond);
        ast_free(tree);
        token_free(tok);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }
    lexer_pop(status, lexer);
    token_free(tok);

    struct ast *then = parse_compound_list(status, lexer);
    if (*status != PARSER_OK || !then)
    {
        if (then)
        {
            ast_free(then);
        }
        *status = PARSER_UNEXPECTED_TOKEN;
        ast_free(tree);
        // ast_free(cond);
        return NULL;
    }

    ast_add_child(tree, then);

    struct token *tok2 = lexer_peek(status, lexer);
    if (tok2->type != DONE)
    {
        // ast_free(cond);
        ast_free(tree);
        token_free(tok2);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }
    lexer_pop(status, lexer);
    token_free(tok2);
    tok = lexer_peek(status, lexer);
    if (tok->type == REDIRECTION)
    {
        lexer_pop(status, lexer);
        struct token *tok2 = lexer_peek(status, lexer);
        if (tok2->type != FUNCTION)
        {
            token_free(tok);
            token_free(tok2);
            ast_free(tree);
            *status = PARSER_UNEXPECTED_TOKEN;
            return NULL;
        }
        struct ast *new = ast_new(FILEM);
        new->value = malloc(100000);
        strcpy(new->value, tok2->value);
        struct ast *new2 = ast_new(REDIRECTION);
        new2->value = malloc(100000);
        strcpy(new2->value, tok->value);
        ast_add_child(new, new2);
        ast_add_child(new, tree);

        lexer_pop(status, lexer);
        token_free(tok);
        token_free(tok2);
        return new;
    }
    token_free(tok);
    return tree;
}*/

static struct ast *parse_rule_until_part1(enum parser_status *status,
                                          struct lexer *lexer)
{
    struct token *tok = lexer_peek(status, lexer);
    if (tok->type != UNTIL)
    {
        token_free(tok);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }
    struct ast *tree = ast_new(UNTIL);

    lexer_pop(status, lexer);
    token_free(tok);

    // condution

    // verifier que c'est bien une FUNUC apres
    // struct token *tok2 = lexer_peek(status, lexer);
    /* if (tok2->type != FUNCTION && tok2->type != VARIABLE
        && tok2->type != NEGATION && tok2->type != EXEC
        && tok2->type != FUNCTION_RIGHT && tok2->type != FUNCTION_LEFT
        && tok2->type != VARIABLE_RIGHT && tok2->type != VARIABLE_LEFT
    && tok2->type != FUNCTION_MIDDLE)
    {
        ast_free(tree);
        token_free(tok2);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
        }
        token_free(tok2);*/

    struct ast *cond = parse_compound_list(status, lexer);
    if (*status != PARSER_OK || !cond)
    {
        *status = PARSER_UNEXPECTED_TOKEN;
        ast_free(tree);
        if (cond)
        {
            ast_free(cond);
        }
        return NULL;
    }

    ast_add_child(tree, cond);

    return parse_rule_until_part2(status, lexer, tree);
}

static struct ast *parse_rule_until(enum parser_status *status,
                                    struct lexer *lexer)
{
    return parse_rule_until_part1(status, lexer);
}

static void parse_rule_for_part2(enum parser_status *status,
                                 struct lexer *lexer, struct ast *list)
{
    struct token *tok = lexer_peek(status, lexer);
    if (tok->type != IN)
    {
        *status = PARSER_UNEXPECTED_TOKEN;
        token_free(tok);
        return;
    }

    lexer->arg_for = 0;

    token_free(tok);
    lexer_pop(status, lexer);
    tok = lexer_peek(status, lexer);
    while (tok->type == FUNCTION || tok->type == VARIABLE)
    {
        struct ast *child = ast_new(tok->type);
        child->value = malloc(strlen(tok->value) + 10);
        strcpy(child->value, tok->value);
        ast_add_child(list, child);
        token_free(tok);
        lexer_pop(status, lexer);
        tok = lexer_peek(status, lexer);
    }
    if (tok->type != EOL)
    {
        token_free(tok);
        *status = PARSER_UNEXPECTED_TOKEN;
        return;
    }
    token_free(tok);
    lexer_pop(status, lexer);
}

static int parse_rule_for_part1_A(enum parser_status *status, struct ast *tree,
                                  struct token **ptok2)
{
    struct token *tok2;
    tok2 = *ptok2;
    if (tok2->type == EOL || tok2->type == EOFF || tok2->string == 1)
    {
        ast_free(tree);
        token_free(tok2);
        *status = PARSER_UNEXPECTED_TOKEN;
        return 1;
    }
    return 0;
}

static int parse_rule_for_part1_B(enum parser_status *status,
                                  struct lexer *lexer, struct ast *tree,
                                  struct token **ptok2)
{
    struct token *tok2;
    tok2 = *ptok2;
    struct ast *list;
    list = ast_new(IN);
    ast_add_child(tree, list);

    if (tok2->type == EOL && tok2->value[0] == 'v')
    {
        token_free(tok2);
        lexer_pop(status, lexer);
        tok2 = lexer_peek(status, lexer);
    }
    else
    {
        while (tok2->type == EOL && tok2->value[0] == 'n')
        {
            token_free(tok2);
            lexer_pop(status, lexer);
            tok2 = lexer_peek(status, lexer);
        }
        if (tok2->type == EOL)
        {
            token_free(tok2);
            ast_free(tree);
            *status = PARSER_UNEXPECTED_TOKEN;
            return 1;
        }
        if (tok2->type == IN)
        {
            token_free(tok2);
            parse_rule_for_part2(status, lexer, list);
            if (*status == PARSER_UNEXPECTED_TOKEN)
            {
                ast_free(tree);
                return 1;
            }
            tok2 = lexer_peek(status, lexer);
        }
    }
    *ptok2 = tok2;
    return 0;
}

static int parse_rule_for_part1_C(enum parser_status *status,
                                  struct lexer *lexer, struct ast *tree,
                                  struct token **ptok2)
{
    struct token *tok2;
    tok2 = *ptok2;
    while (tok2->type == EOL && tok2->value[0] == 'n')
    {
        token_free(tok2);
        lexer_pop(status, lexer);
        tok2 = lexer_peek(status, lexer);
    }

    token_free(tok2);
    tok2 = lexer_peek(status, lexer);
    if (tok2->type != DO)
    {
        ast_free(tree);
        token_free(tok2);
        *status = PARSER_UNEXPECTED_TOKEN;
        return 1;
    }
    token_free(tok2);
    lexer_pop(status, lexer);

    struct ast *cl;
    cl = parse_compound_list(status, lexer);
    if (*status != PARSER_OK || cl == NULL)
    {
        if (cl)
        {
            ast_free(cl);
        }
        *status = PARSER_UNEXPECTED_TOKEN;
        ast_free(tree);
        return 1;
    }
    ast_add_child(tree, cl);
    tok2 = lexer_peek(status, lexer);
    if (tok2->type != DONE)
    {
        ast_free(tree);
        token_free(tok2);
        *status = PARSER_UNEXPECTED_TOKEN;
        return 1;
    }
    token_free(tok2);
    lexer_pop(status, lexer);
    return 0;
}

static struct ast *parse_rule_for_part1(enum parser_status *status,
                                        struct lexer *lexer)
{
    struct token *tok;
    tok = lexer_peek(status, lexer);
    if (tok->type != FOR)
    {
        token_free(tok);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }
    struct ast *tree;
    tree = ast_new(FOR);

    lexer_pop(status, lexer);
    token_free(tok);

    struct token *tok2;
    tok2 = lexer_peek(status, lexer);

    int retA;
    retA = parse_rule_for_part1_A(status, tree, &tok2);
    if (retA == 1)
    {
        return NULL;
    }

    struct ast *eq;
    eq = ast_new(EQUAL);
    ast_add_child(tree, eq);

    struct ast *var;
    var = ast_new(FUNCTION);
    var->value = malloc(sizeof(tok2->value));
    strcpy(var->value, tok2->value);
    ast_add_child(eq, var);
    token_free(tok2);

    struct ast *val;
    val = ast_new(FUNCTION);
    val->value = calloc(1, 1);
    ast_add_child(eq, val);

    lexer_pop(status, lexer);
    tok2 = lexer_peek(status, lexer);

    int retB;
    retB = parse_rule_for_part1_B(status, lexer, tree, &tok2);
    if (retB == 1)
    {
        return NULL;
    }

    int retC;
    retC = parse_rule_for_part1_C(status, lexer, tree, &tok2);
    if (retC == 1)
    {
        return NULL;
    }

    return tree;
}

/*
static struct ast *parse_rule_for_part1(enum parser_status *status,
                                        struct lexer *lexer)
{
    struct token *tok = lexer_peek(status, lexer);
    if (tok->type != FOR)
    {
        token_free(tok);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }
    struct ast *tree = ast_new(FOR);

    lexer_pop(status, lexer);
    token_free(tok);

    // condution

    // verifier que c'est bien une FUNC apres
    struct token *tok2 = lexer_peek(status, lexer);
    if (tok2->type == EOL || tok2->type == EOFF || tok2->string == 1)
    {
        ast_free(tree);
        token_free(tok2);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }

    struct ast *eq = ast_new(EQUAL);
    ast_add_child(tree, eq);

    struct ast *var = ast_new(FUNCTION);
    var->value = malloc(sizeof(tok2->value));
    strcpy(var->value, tok2->value);
    ast_add_child(eq, var);
    token_free(tok2);

    struct ast *val = ast_new(FUNCTION);
    val->value = calloc(1, 1);
    ast_add_child(eq, val);

    lexer_pop(status, lexer);
    tok2 = lexer_peek(status, lexer);
    struct ast *list = ast_new(IN);
    ast_add_child(tree, list);
    if (tok2->type == EOL && tok2->value[0] == 'v')
    {
        token_free(tok2);
        lexer_pop(status, lexer);
        tok2 = lexer_peek(status, lexer);
    }
    else
    {
        while (tok2->type == EOL && tok2->value[0] == 'n')
        {
            token_free(tok2);
            lexer_pop(status, lexer);
            tok2 = lexer_peek(status, lexer);
        }
        if (tok2->type == EOL)
        {
            token_free(tok2);
            ast_free(tree);
            *status = PARSER_UNEXPECTED_TOKEN;
            return NULL;
        }
        if (tok2->type == IN)
        {
            token_free(tok2);
            parse_rule_for_part2(status, lexer, list);
            if (*status == PARSER_UNEXPECTED_TOKEN)
            {
                ast_free(tree);
                return NULL;
            }
            tok2 = lexer_peek(status, lexer);
        }
    }

    while (tok2->type == EOL && tok2->value[0] == 'n')
    {
        token_free(tok2);
        lexer_pop(status, lexer);
        tok2 = lexer_peek(status, lexer);
    }

    token_free(tok2);
    tok2 = lexer_peek(status, lexer);
    if (tok2->type != DO)
    {
        ast_free(tree);
        token_free(tok2);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }
    token_free(tok2);
    lexer_pop(status, lexer);
    struct ast *cl = parse_compound_list(status, lexer);
    if (*status != PARSER_OK || cl == NULL)
    {
        if (cl)
        {
            ast_free(cl);
        }
        *status = PARSER_UNEXPECTED_TOKEN;
        ast_free(tree);
        return NULL;
    }
    ast_add_child(tree, cl);
    tok2 = lexer_peek(status, lexer);
    if (tok2->type != DONE)
    {
        ast_free(tree);
        token_free(tok2);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }

    token_free(tok2);
    lexer_pop(status, lexer);
    return tree;
}*/

static struct ast *parse_rule_for(enum parser_status *status,
                                  struct lexer *lexer)
{
    return parse_rule_for_part1(status, lexer);
}

static struct ast *parse_shell_command(enum parser_status *status,
                                       struct lexer *lexer)
{
    struct token *tok = lexer_peek(status, lexer);
    struct ast *left = NULL;

    if (tok->type == IF)
    {
        token_free(tok);
        left = parse_rule_if(status, lexer);
    }
    else if (tok->type == WHILE)
    {
        token_free(tok);
        left = parse_rule_while(status, lexer);
    }
    else if (tok->type == UNTIL)
    {
        token_free(tok);
        left = parse_rule_until(status, lexer);
    }
    else if (tok->type == FOR)
    {
        token_free(tok);
        left = parse_rule_for(status, lexer);
    }
    else
    {
        token_free(tok);
        return NULL;
    }

    if (*status != PARSER_OK || left == NULL)
    {
        if (left)
        {
            ast_free(left);
        }
        return NULL;
    }

    return left;
}

static struct ast *parse_command(enum parser_status *status,
                                 struct lexer *lexer)
{
    struct token *tok = lexer_peek(status, lexer);
    if (tok->type == IF || tok->type == UNTIL || tok->type == WHILE
        || tok->type == FOR)
    {
        token_free(tok);
        return parse_shell_command(status, lexer);
    }
    else
    {
        token_free(tok);
        return parse_simple_command(status, lexer);
    }
}

struct list_chainer_token
{
    struct token *value;
    struct list_chainer_token *next;
};

static void list_chainer_token_free(struct list_chainer_token *list)
{
    while (list != NULL)
    {
        struct list_chainer_token *list2 = list;
        list = list->next;
        token_free(list2->value);
        free(list2);
    }
}

static int is_a_var(enum parser_status *status, struct token *tok,
                    struct lexer *lexer)
{
    if (tok->type != FUNCTION)
    {
        return 0;
    }
    // token_free(tok);
    size_t tmp = lexer->pos;
    if (tmp == 0)
    {
        lexer->arg2 = 0;
    }
    lexer_pop(status, lexer);
    tok = lexer_peek(status, lexer);
    if (tok->type != EQUAL && tok->type != VARIABLE_SUPP)
    {
        token_free(tok);
        lexer->pos = tmp;
        return 0;
    }
    if (tok->type == VARIABLE_SUPP)
    {
        token_free(tok);
        lexer_pop(status, lexer);
        tok = lexer_peek(status, lexer);
        while (tok->type == EOL)
        {
            token_free(tok);
            lexer->input[lexer->pos - 1] = ' ';
            lexer_pop(status, lexer);
            tok = lexer_peek(status, lexer);
        }
        token_free(tok);

        return 2;
    }

    token_free(tok);
    lexer_pop(status, lexer);
    lexer_pop(status, lexer);
    tok = lexer_peek(status, lexer);
    if (tok->type != EOL && tok->type != EOFF)
    {
        token_free(tok);
        return 2;
    }
    while (tok->type == EOL)
    {
        lexer->input[lexer->pos] = ' ';
        token_free(tok);
        lexer_pop(status, lexer);
        tok = lexer_peek(status, lexer);
    }
    token_free(tok);
    lexer->pos = tmp;
    return 1;
}

static struct list_chainer_token *parse_var(struct ast *pere,
                                            struct list_chainer_token *list)
{
    struct ast *node3 = ast_new(list->value->type);
    if (list->value->value != NULL)
    {
        node3->value = malloc(100000);
        strcpy(node3->value, list->value->value);
    }
    list = list->next;
    struct ast *node4 = ast_new(list->value->type);
    if (list->value->value != NULL)
    {
        node4->value = malloc(100000);
        strcpy(node4->value, list->value->value);
    }
    list = list->next;
    struct ast *node5 = ast_new(list->value->type);
    if (list->value->value != NULL)
    {
        node5->value = malloc(100000);
        strcpy(node5->value, list->value->value);
    }
    list = list->next;
    ast_add_child(node3, node4);
    ast_add_child(node3, node5);
    ast_add_child(pere, node3);
    return list;
}

/*


struct list_chainer_token *constuct_while_1(enum parser_status *status,
                                    struct lexer *lexer)
{
    while (tok->type != EOL && tok->type != EOFF && tok->type != OR
           && tok->type != AND && tok->type != PIPE && tok->type != NEGATION)
    {
        if ((stop == 1 && tok->type == FUNCTION)
            || (stop == 1 && tok->type == VARIABLE))
        {
            stop = 0;
            token_free(tok);
            lexer_pop(status, lexer);
            tok = lexer_peek(status, lexer);
            continue;
        }
        int i = is_a_var(status, tok, lexer);

        // tok = lexer_peek(status, lexer);
        if (i == 1)
        {
            token_free(tok);
            struct token *tok2 = lexer_peek(status, lexer);
            lexer_pop(status, lexer);
            struct token *tok3 = lexer_peek(status, lexer);
            lexer_pop(status, lexer);
            struct token *tok4 = lexer_peek(status, lexer);
            if (test != 0)
            {
                list->next = malloc(sizeof(struct list_chainer_token));
                list = list->next;
            }
            list->value = tok3;
            list->next = malloc(sizeof(struct list_chainer_token));
            list = list->next;
            list->value = tok2;
            list->next = malloc(sizeof(struct list_chainer_token));
            list = list->next;
            list->value = tok4;
            list->next = NULL;
            test = 1;
            func = 1;
            stop = 0;
        }
        else if (i == 2)
        {
            token_free(tok);
            tok = lexer_peek(status, lexer);
            tmp = lexer->pos;
            continue;
        }
        else if (tok->type != IONUMBER && tok->type != REDIRECTION)
        {
            if (test != 0)
            {
                list->next = malloc(sizeof(struct list_chainer_token));
                list = list->next;
            }
            list->value = tok;
            func = 1;
            list->next = NULL;
            stop = 0;
            test = 1;
        }
        else if (tok->type == REDIRECTION && stop == 1)
        {
            list_chainer_token_free(list);
            token_free(tok);
            *status = PARSER_UNEXPECTED_TOKEN;
            return NULL;
        }
        else if (tok->type == REDIRECTION)
        {
            token_free(tok);
            stop = 1;
        }
        else
        {
            stop = 0;
            token_free(tok);
        }
        lexer_pop(status, lexer);
        tok = lexer_peek(status, lexer);
    }
}

*/

struct construct_W
{
    size_t tmp;
    struct list_chainer_token *list;
    struct list_chainer_token *res;
    struct token *tok;
    size_t stop;
    size_t test;
    size_t func;
};

static struct construct_W *init(enum parser_status *status, struct lexer *lexer)
{
    struct construct_W *ini;
    ini = malloc(sizeof(struct construct_W));

    ini->tmp = lexer->pos;
    ini->list = malloc(sizeof(struct list_chainer_token));
    ini->res = ini->list;

    ini->tok = lexer_peek(status, lexer);

    ini->stop = 0;
    ini->test = 0;
    ini->func = 0;

    return ini;
}

static void handle_i_equals_1(struct construct_W *ini,
                              enum parser_status *status, struct lexer *lexer)
{
    token_free(ini->tok);

    struct token *tok2;
    struct token *tok3;
    struct token *tok4;

    tok2 = lexer_peek(status, lexer);
    lexer_pop(status, lexer);

    tok3 = lexer_peek(status, lexer);
    lexer_pop(status, lexer);

    tok4 = lexer_peek(status, lexer);

    if (ini->test != 0)
    {
        ini->list->next = malloc(sizeof(struct list_chainer_token));
        ini->list = ini->list->next;
    }
    ini->list->value = tok3;
    ini->list->next = malloc(sizeof(struct list_chainer_token));
    ini->list = ini->list->next;
    ini->list->value = tok2;
    ini->list->next = malloc(sizeof(struct list_chainer_token));
    ini->list = ini->list->next;
    ini->list->value = tok4;
    ini->list->next = NULL;
    ini->test = 1;
    ini->func = 1;
    ini->stop = 0;
}

static int handle_i_equals_2(struct construct_W *ini,
                             enum parser_status *status)
{
    list_chainer_token_free(ini->list);
    token_free(ini->tok);
    *status = PARSER_UNEXPECTED_TOKEN;
    return 1;
}

static int construct_main_while(struct construct_W *ini,
                                enum parser_status *status, struct lexer *lexer)
{
    while (ini->tok->type != EOL && ini->tok->type != EOFF
           && ini->tok->type != OR && ini->tok->type != AND
           && ini->tok->type != PIPE && ini->tok->type != NEGATION)
    {
        if ((ini->stop == 1 && ini->tok->type == FUNCTION)
            || (ini->stop == 1 && ini->tok->type == VARIABLE))
        {
            ini->stop = 0;
            token_free(ini->tok);
            lexer_pop(status, lexer);
            ini->tok = lexer_peek(status, lexer);
            continue;
        }

        int i;
        i = is_a_var(status, ini->tok, lexer);
        if (i == 1)
        {
            handle_i_equals_1(ini, status, lexer);
        }
        else if (i == 2)
        {
            token_free(ini->tok);
            ini->tok = lexer_peek(status, lexer);
            ini->tmp = lexer->pos;
            continue;
        }
        else if (ini->tok->type != IONUMBER && ini->tok->type != REDIRECTION)
        {
            if (ini->test != 0)
            {
                ini->list->next = malloc(sizeof(struct list_chainer_token));
                ini->list = ini->list->next;
            }
            ini->list->value = ini->tok;
            ini->func = 1;
            ini->list->next = NULL;
            ini->stop = 0;
            ini->test = 1;
        }
        else if (ini->tok->type == REDIRECTION && ini->stop == 1)
        {
            return handle_i_equals_2(ini, status);
        }
        else if (ini->tok->type == REDIRECTION)
        {
            token_free(ini->tok);
            ini->stop = 1;
        }
        else
        {
            ini->stop = 0;
            token_free(ini->tok);
        }

        lexer_pop(status, lexer);
        ini->tok = lexer_peek(status, lexer);
    }
    return 0;
}

static int construct_second_while_loop(struct construct_W *ini,
                                       enum parser_status *status,
                                       struct lexer *lexer,
                                       struct list_chainer_token **list2_ptr)
{
    struct list_chainer_token *list2;
    list2 = *list2_ptr;

    while (ini->tok->type != EOL && ini->tok->type != EOFF
           && ini->tok->type != AND && ini->tok->type != OR
           && ini->tok->type != PIPE && ini->tok->type != NEGATION)
    {
        if ((ini->stop == 1 && ini->tok->type == FUNCTION)
            || (ini->stop == 1 && ini->tok->type == VARIABLE))
        {
            struct list_chainer_token *pas;
            pas = malloc(sizeof(struct list_chainer_token));
            pas->value = ini->tok;
            pas->value->type = FILEM;
            pas->next = list2;
            list2 = pas;
        }
        else if (ini->tok->type == IONUMBER || ini->tok->type == REDIRECTION)
        {
            struct list_chainer_token *pas;
            pas = malloc(sizeof(struct list_chainer_token));
            if (ini->func == 0)
            {
                free(list2);
                free(ini->list);
                free(ini->res);
                list2 = NULL;
            }
            pas->value = ini->tok;
            ini->func = 1;
            pas->next = list2;
            list2 = pas;
            if (ini->tok->type == REDIRECTION)
            {
                ini->stop = 1;
                ini->test = 1;
            }
            else
            {
                ini->stop = 0;
            }
        }
        else
        {
            token_free(ini->tok);
        }
        lexer_pop(status, lexer);
        ini->tok = lexer_peek(status, lexer);
        ini->res = list2;
    }
    *list2_ptr = list2;
    return 0;
}

static int construct_second_while(struct construct_W *ini,
                                  enum parser_status *status,
                                  struct lexer *lexer)
{
    token_free(ini->tok);

    if (ini->stop == 1)
    {
        list_chainer_token_free(ini->list);
        *status = PARSER_UNEXPECTED_TOKEN;
        return 1;
    }

    struct list_chainer_token *list2;
    list2 = ini->res;

    if (ini->test == 0)
    {
        list2 = NULL;
        free(ini->list);
    }

    ini->stop = 0;
    lexer->pos = ini->tmp;

    lexer_pop(status, lexer);
    ini->tok = lexer_peek(status, lexer);

    int ret;
    ret = construct_second_while_loop(ini, status, lexer, &list2);
    if (ret != 0)
    {
        return ret;
    }

    token_free(ini->tok);

    if (ini->test == 0)
    {
        return 2;
    }

    return 0;
}

struct list_chainer_token *constuct(enum parser_status *status,
                                    struct lexer *lexer)
{
    struct construct_W *ini = init(status, lexer);

    int r;
    r = construct_main_while(ini, status, lexer);
    if (r == 1)
    {
        free(ini);
        return NULL;
    }

    int r2;
    r2 = construct_second_while(ini, status, lexer);
    if (r2 == 1)
    {
        free(ini);
        return NULL;
    }
    if (r2 == 2)
    {
        free(ini);
        return NULL;
    }

    struct list_chainer_token *final_res;
    final_res = ini->res;

    free(ini);
    return final_res;
}

/*

struct list_chainer_token *constuct(enum parser_status *status,
                                    struct lexer *lexer)
{
    size_t tmp = lexer->pos;
    struct list_chainer_token *list = malloc(sizeof(struct list_chainer_token));
    struct list_chainer_token *res = list;
    struct token *tok = lexer_peek(status, lexer);
    size_t stop = 0;
    size_t test = 0;
    size_t func = 0;
    while (tok->type != EOL && tok->type != EOFF && tok->type != OR
           && tok->type != AND && tok->type != PIPE && tok->type != NEGATION)
    {
        if ((stop == 1 && tok->type == FUNCTION)
            || (stop == 1 && tok->type == VARIABLE))
        {
            stop = 0;
            token_free(tok);
            lexer_pop(status, lexer);
            tok = lexer_peek(status, lexer);
            continue;
        }
        int i = is_a_var(status, tok, lexer);

        // tok = lexer_peek(status, lexer);
        if (i == 1)
        {
            token_free(tok);
            struct token *tok2 = lexer_peek(status, lexer);
            lexer_pop(status, lexer);
            struct token *tok3 = lexer_peek(status, lexer);
            lexer_pop(status, lexer);
            struct token *tok4 = lexer_peek(status, lexer);
            if (test != 0)
            {
                list->next = malloc(sizeof(struct list_chainer_token));
                list = list->next;
            }
            list->value = tok3;
            list->next = malloc(sizeof(struct list_chainer_token));
            list = list->next;
            list->value = tok2;
            list->next = malloc(sizeof(struct list_chainer_token));
            list = list->next;
            list->value = tok4;
            list->next = NULL;
            test = 1;
            func = 1;
            stop = 0;
        }
        else if (i == 2)
        {
            token_free(tok);
            tok = lexer_peek(status, lexer);
            tmp = lexer->pos;
            continue;
        }
        else if (tok->type != IONUMBER && tok->type != REDIRECTION)
        {
            if (test != 0)
            {
                list->next = malloc(sizeof(struct list_chainer_token));
                list = list->next;
            }
            list->value = tok;
            func = 1;
            list->next = NULL;
            stop = 0;
            test = 1;
        }
        else if (tok->type == REDIRECTION && stop == 1)
        {
            list_chainer_token_free(list);
            token_free(tok);
            *status = PARSER_UNEXPECTED_TOKEN;
            return NULL;
        }
        else if (tok->type == REDIRECTION)
        {
            token_free(tok);
            stop = 1;
        }
        else
        {
            stop = 0;
            token_free(tok);
        }
        lexer_pop(status, lexer);
        tok = lexer_peek(status, lexer);
    }
    token_free(tok);
    if (stop == 1)
    {
        list_chainer_token_free(list);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }
    struct list_chainer_token *list2 = res;
    if (test == 0)
    {
        list2 = NULL;
        free(list);
    }
    stop = 0;
    lexer->pos = tmp;

    lexer_pop(status, lexer);
    tok = lexer_peek(status, lexer);
    while (tok->type != EOL && tok->type != EOFF && tok->type != AND
           && tok->type != OR && tok->type != PIPE && tok->type != NEGATION)
    {
        if ((stop == 1 && tok->type == FUNCTION)
            || (stop == 1 && tok->type == VARIABLE))
        {
            struct list_chainer_token *pas =
                malloc(sizeof(struct list_chainer_token));
            pas->value = tok;
            pas->value->type = FILEM;
            pas->next = list2;

            list2 = pas;
        }
        else if (tok->type == IONUMBER || tok->type == REDIRECTION)
        {
            struct list_chainer_token *pas =
                malloc(sizeof(struct list_chainer_token));
            if (func == 0)
            {
                free(list2);
                free(list);
                free(res);
                list2 = NULL;
            }
            pas->value = tok;
            func = 1;
            pas->next = list2;
            list2 = pas;
            if (tok->type == REDIRECTION)
            {
                stop = 1;
                test = 1;
            }
            else
            {
                stop = 0;
            }
        }
        else
        {
            token_free(tok);
        }
        lexer_pop(status, lexer);
        tok = lexer_peek(status, lexer);
        res = list2;
    }
    token_free(tok);
    if (test == 0)
    {
        return NULL;
    }
    return res;
}

*/

struct parse_scp2_ctx
{
    enum parser_status *status;
    struct lexer *lexer;
    struct ast *tree;
    struct list_chainer_token *list;
    struct list_chainer_token *res;
    size_t stop;
    struct ast *pere;
};

static int handle_case_1(struct parse_scp2_ctx *ctx)
{
    struct token *val;
    val = ctx->list->value;

    if ((val->type == FUNCTION_LEFT && ctx->stop == 1)
        || (val->type == VARIABLE_RIGHT && ctx->stop == 1)
        || (val->type == FUNCTION_RIGHT && ctx->stop == 1)
        || (val->type == VARIABLE_LEFT && ctx->stop == 1)
        || (val->type == FUNCTION_MIDDLE && ctx->stop == 1)
        || (val->type == FUNCTION && ctx->stop == 1)
        || (val->type == VARIABLE && ctx->stop == 1)
        || (val->type == EXEC && ctx->stop == 1))
    {
        struct ast *node3;
        node3 = ast_new(val->type);
        if (val->value != NULL)
        {
            node3->value = malloc(100000);
            strcpy(node3->value, val->value);
        }
        ctx->list = ctx->list->next;
        ast_add_child(ctx->pere, node3);
        ctx->pere = node3;
        ctx->stop = 0;
        return 1;
    }
    return 0;
}

static int handle_case_2(struct parse_scp2_ctx *ctx)
{
    struct token *val;
    val = ctx->list->value;

    if (val->type == FUNCTION || val->type == VARIABLE
        || val->type == FUNCTION_LEFT || val->type == FUNCTION_RIGHT
        || val->type == VARIABLE_RIGHT || val->type == FUNCTION_MIDDLE
        || val->type == VARIABLE_LEFT || val->type == EXEC)
    {
        struct ast *node3;
        node3 = ast_new(val->type);
        if (val->value != NULL)
        {
            node3->value = malloc(100000);
            strcpy(node3->value, val->value);
        }
        ctx->list = ctx->list->next;
        ast_add_child(ctx->pere, node3);
        ctx->stop = 0;
        return 1;
    }
    return 0;
}

static int handle_case_3(struct parse_scp2_ctx *ctx)
{
    struct token *val;
    val = ctx->list->value;

    if (val->type == IONUMBER)
    {
        struct ast *node3;
        node3 = ast_new(IONUMBER);
        if (val->value != NULL)
        {
            node3->value = malloc(100000);
            strcpy(node3->value, val->value);
        }
        ctx->list = ctx->list->next;
        ast_add_child(ctx->pere, node3);
        return 1;
    }
    return 0;
}

static int handle_case_4(struct parse_scp2_ctx *ctx)
{
    struct token *val;
    val = ctx->list->value;

    if (val->type == EQUAL)
    {
        ctx->list = parse_var(ctx->pere, ctx->list);
        ctx->stop = 1;
        return 1;
    }
    return 0;
}

static int handle_case_5(struct parse_scp2_ctx *ctx)
{
    struct token *val;
    val = ctx->list->value;

    if (val->type == REDIRECTION)
    {
        struct ast *node3;
        node3 = ast_new(REDIRECTION);
        if (val->value != NULL)
        {
            node3->value = malloc(100000);
            strcpy(node3->value, val->value);
        }
        ctx->list = ctx->list->next;
        ast_add_child(ctx->pere, node3);
        ctx->stop = 1;
        return 1;
    }
    return 0;
}

static int handle_case_6(struct parse_scp2_ctx *ctx)
{
    struct token *val;
    val = ctx->list->value;

    if (val->type == FILEM)
    {
        struct ast *node3;
        node3 = ast_new(FILEM);
        if (val->value != NULL)
        {
            node3->value = malloc(100000);
            strcpy(node3->value, val->value);
        }
        ctx->list = ctx->list->next;
        ast_add_child(ctx->pere, node3);
        ctx->pere = node3;
        ctx->stop = 0;
        return 1;
    }
    return 0;
}

static void handle_else_default(struct parse_scp2_ctx *ctx)
{
    ctx->list = ctx->list->next;
}

static void parse_simple_command_part2_while(struct parse_scp2_ctx *ctx)
{
    while (ctx->list != NULL)
    {
        int done;
        done = handle_case_1(ctx);
        if (done == 1)
        {
            continue;
        }

        done = handle_case_2(ctx);
        if (done == 1)
        {
            continue;
        }

        done = handle_case_3(ctx);
        if (done == 1)
        {
            continue;
        }

        done = handle_case_4(ctx);
        if (done == 1)
        {
            continue;
        }

        done = handle_case_5(ctx);
        if (done == 1)
        {
            continue;
        }

        done = handle_case_6(ctx);
        if (done == 1)
        {
            continue;
        }

        handle_else_default(ctx);
    }
}

static int parse_simple_command_part2_init(struct parse_scp2_ctx *ctx)
{
    ctx->tree = ast_new(AST_LIST);
    ctx->list = constuct(ctx->status, ctx->lexer);

    if (*ctx->status != PARSER_OK || ctx->list == NULL)
    {
        if (ctx->list)
        {
            list_chainer_token_free(ctx->list);
        }
        ast_free(ctx->tree);
        return 1;
    }
    ctx->res = ctx->list;
    ctx->stop = 1;
    return 0;
}

static struct ast *parse_simple_command_part2_finish(struct parse_scp2_ctx *ctx)
{
    list_chainer_token_free(ctx->res);
    return ctx->tree;
}

static struct ast *parse_simple_command_part2(enum parser_status *status,
                                              struct lexer *lexer)
{
    struct parse_scp2_ctx c;
    c.status = status;
    c.lexer = lexer;
    c.tree = NULL;
    c.list = NULL;
    c.res = NULL;
    c.stop = 1;
    c.pere = NULL;

    int r;
    r = parse_simple_command_part2_init(&c);
    if (r == 1)
    {
        return NULL;
    }

    if (c.list != NULL)
    {
        struct ast *pere;
        pere = ast_new(AST_LIST);
        ast_add_child(c.tree, pere);
        c.pere = pere;

        parse_simple_command_part2_while(&c);
    }

    return parse_simple_command_part2_finish(&c);
}

/*
static struct ast *parse_simple_command_part2(enum parser_status *status,
                                              struct lexer *lexer)
{
    struct ast *tree = ast_new(AST_LIST);

    struct list_chainer_token *list = constuct(status, lexer);

    if (*status != PARSER_OK || list == NULL)
    {
        if (list)
        {
            list_chainer_token_free(list);
        }
        ast_free(tree);
        return NULL;
    }
    struct list_chainer_token *res = list;

    size_t stop = 1;
    if (list != NULL)
    {
        struct ast *pere = ast_new(AST_LIST);
        ast_add_child(tree, pere);

        while (list != NULL)
        {
            if ((list->value->type == FUNCTION_LEFT && stop == 1)
                || (list->value->type == VARIABLE_RIGHT && stop == 1)
                || (list->value->type == FUNCTION_RIGHT && stop == 1)
                || (list->value->type == VARIABLE_LEFT && stop == 1)
                || (list->value->type == FUNCTION_MIDDLE && stop == 1)
                || (list->value->type == FUNCTION && stop == 1)
                || (list->value->type == VARIABLE && stop == 1)
                || (list->value->type == EXEC && stop == 1))
            {
                struct ast *node3 = ast_new(list->value->type);
                if (list->value->value != NULL)
                {
                    node3->value = malloc(100000);
                    strcpy(node3->value, list->value->value);
                }

                list = list->next;

                ast_add_child(pere, node3);

                pere = node3;

                stop = 0;
            }
            else if (list->value->type == FUNCTION
                     || list->value->type == VARIABLE
                     || list->value->type == FUNCTION_LEFT
                     || list->value->type == FUNCTION_RIGHT
                     || list->value->type == VARIABLE_RIGHT
                     || list->value->type == FUNCTION_MIDDLE
                     || list->value->type == VARIABLE_LEFT
                     || list->value->type == EXEC)
            {
                struct ast *node3 = NULL;
                node3 = ast_new(list->value->type);
                if (list->value->value != NULL)
                {
                    node3->value = malloc(100000);
                    strcpy(node3->value, list->value->value);
                }

                list = list->next;

                ast_add_child(pere, node3);

                stop = 0;
            }
            else if (list->value->type == IONUMBER)
            {
                struct ast *node3 = ast_new(list->value->type);
                if (list->value->value != NULL)
                {
                    node3->value = malloc(100000);
                    strcpy(node3->value, list->value->value);
                }

                list = list->next;

                ast_add_child(pere, node3);
            }
            else if (list->value->type == EQUAL)
            {
                list = parse_var(pere, list);
                // lexer_pop(status, lexer);
                // lexer_pop(status, lexer);
                stop = 1;
            }
            else if (list->value->type == REDIRECTION)
            {
                struct ast *node3 = ast_new(list->value->type);
                if (list->value->value != NULL)
                {
                    node3->value = malloc(100000);
                    strcpy(node3->value, list->value->value);
                }

                list = list->next;

                ast_add_child(pere, node3);

                stop = 1;
            }
            else if (list->value->type == FILEM)
            {
                struct ast *node3 = ast_new(list->value->type);
                if (list->value->value != NULL)
                {
                    node3->value = malloc(100000);
                    strcpy(node3->value, list->value->value);
                }

                list = list->next;

                ast_add_child(pere, node3);
                pere = node3;
                stop = 0;
            }
            else
            {
                list = list->next;
            }
            // lexer_pop(status, lexer);
        }
    }
    // list_chainer_token_free(list);
    list_chainer_token_free(res);
    // print_ast2(tree, 0);
    return tree;
}
*/

static struct ast *parse_simple_command(enum parser_status *status,
                                        struct lexer *lexer)
{
    struct token *tok = lexer_peek(status, lexer);

    if (tok->type != FUNCTION && tok->type != IONUMBER
        && tok->type != REDIRECTION && tok->type != FUNCTION_RIGHT
        && tok->type != FUNCTION_LEFT && tok->type != FUNCTION_MIDDLE
        && tok->type != VARIABLE && tok->type != VARIABLE_RIGHT
        && tok->type != VARIABLE_LEFT && tok->type != EXEC)
    {
        token_free(tok);
        *status = PARSER_UNEXPECTED_TOKEN;
        return NULL;
    }
    token_free(tok);
    return parse_simple_command_part2(status, lexer);
}

/*static struct ast *parse_simple_command_part1(enum parser_status *status,
                                              struct lexer *lexer,
                                              struct ast *tree);

static struct ast *parse_simple_command_part2(enum parser_status *status,
                                              struct lexer *lexer,
                                              struct ast *tree)
{
    struct token *next = NULL;
    while (1)
    {
        next = lexer_peek(status, lexer);
        if (next->type != FUNCTION && next->type != VARIABLE
            && next->type != EXEC && next->type != FUNCTION_RIGHT
            && next->type != FUNCTION_LEFT && next->type != VARIABLE_RIGHT
            && next->type != VARIABLE_LEFT && next->type != FUNCTION_MIDDLE)
        {
            while (next->type == EOL && next->value[0] == 'n')
             {
             lexer_pop(status,lexer);
             token_free(next);
             next = lexer_peek(status,lexer);
             }
token_free(next);
            break;
        }

        struct ast *new = parse_element(status, lexer);
        if (*status != PARSER_OK || new == NULL)
        {
            if (new)
            {
                ast_free(new);
            }
            *status = PARSER_UNEXPECTED_TOKEN;
            token_free(next);
            ast_free(tree);
            return NULL;
        }
        token_free(next);

        if (tree->nb_children == tree->capacity)
        {
            size_t newcap = tree->capacity * 2;
            struct ast **tmp =
                realloc(tree->children, newcap * sizeof(struct ast *));
            if (!tmp)
            {
                ast_free(tree);
                *status = PARSER_UNEXPECTED_TOKEN;
                return NULL;
            }
            tree->children = tmp;
            tree->capacity = newcap;
        }

        tree->children[tree->nb_children] = new;
        tree->nb_children++;
    }

    return tree;
}*/

/*static struct ast *parse_element(enum parser_status *status,
                                 struct lexer *lexer)
{
    struct token *tok = lexer_peek(status, lexer);
    if (tok->type != FUNCTION && tok->type != VARIABLE && tok->type != EXEC
        && tok->type != FUNCTION_RIGHT && tok->type != FUNCTION_LEFT
        && tok->type != VARIABLE_RIGHT && tok->type != VARIABLE_LEFT
    && tok->type != FUNCTION_MIDDLE)
    {
        token_free(tok);
        return NULL;
    }

    if (*status == PARSER_UNEXPECTED_TOKEN)
    {
        token_free(tok);
        return NULL;
    }

    // lexer_pop(lexer);

    // printf("%d, %d\n", *status == PARSER_UNEXPECTED_TOKEN, tok->type ==
    // EOFF);

    struct ast *node;
    if (tok->type != EXEC)
    {
        node = ast_new(tok->type);
    }
    else
    {
        node = ast_new(EXEC);
    }
    node->value = malloc(100000);
    if (tok->value != NULL)
    {
        strcpy(node->value, tok->value);
    }
    // node->value = tok->value;
    token_free(tok);
    lexer_pop(status, lexer);
    // token_free(tok);
    return node;
    }*/
