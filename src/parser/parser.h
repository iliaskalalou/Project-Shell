#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"

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

/**
 *
 *      E : | if C then E D fi ; W
 *          | FUNC ; W
 *
 */

// struct ast *parse_E(enum parser_status *status, struct lexer *lexer);

/**
 *
 *      C : | E
 *
 *
 */
// struct ast *parse_C(enum parser_status *status, struct lexer *lexer);

/**
 *
 *      D : | else ; E
 *          | elif C then E D
 *          | null
 *
 */
// struct ast *parse_D(enum parser_status *status, struct lexer *lexer);

/**
 *
 *      W : | E
 *          | null
 *
 */
// struct ast *parse_W(enum parser_status *status, struct lexer *lexer);

struct ast *parse(enum parser_status *status, struct lexer *lexer);

void ast_add_child(struct ast *tree, struct ast *child);

struct ast *parse_list(enum parser_status *status, struct lexer *lexer);

struct ast *parse_compound_list(enum parser_status *status,
                                struct lexer *lexer);
/*
 struct ast *parse_and_or(enum parser_status *status, struct lexer *lexer);

 struct ast *parse_pipeline(enum parser_status *status, struct lexer *lexer);

 struct ast *parse_else_clause(enum parser_status *status, struct lexer *lexer);

 struct ast *parse_rule_if(enum parser_status *status, struct lexer *lexer);

 struct ast *parse_command(enum parser_status *status, struct lexer *lexer);

struct ast *parse_simple_command(enum parser_status *status,
                                 struct lexer *lexer);

struct ast *parse_element(enum parser_status *status, struct lexer *lexer);
*/
#endif /* !PARSER_H */
