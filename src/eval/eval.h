#ifndef EVAL_H
#define EVAL_H

#include "ast.h"

struct special_vars
{
    int last_value;
    char **args;
    size_t nb_args;
};

enum exit_status
{
    RUNNING,
    EXIT
};

int true(void);
int false(void);
int echo(char *args);
int cd(char *path);
int export(char *arg);
int dot(char *file, struct special_vars *special);
int unset(struct ast *A);

int eval_ast(struct ast *A, struct special_vars *special,
             enum exit_status *status);

#endif /* !EVAL_H */
