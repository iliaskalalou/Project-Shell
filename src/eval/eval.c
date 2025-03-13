#define _POSIX_C_SOURCE 200809L

#include "eval.h"

#include <ctype.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static char *build_echo_string(struct ast *A, struct special_vars *special);
int eval_redir(struct ast *node);

static int is_all_digits(const char *var)
{
    for (size_t i = 0; i < strlen(var); i++)
    {
        if (!isdigit((unsigned char)var[i]))
            return 0;
    }
    return 1;
}

static char *get_last_value_str(struct special_vars *special)
{
    char *last_value = malloc(10);
    snprintf(last_value, 10, "%d", special->last_value);
    return last_value;
}

static char *get_nb_args_str(struct special_vars *special)
{
    char *nb_args = malloc(10);
    snprintf(nb_args, 10, "%ld", special->nb_args);
    return nb_args;
}

static char *join_all_args(struct special_vars *special)
{
    char *full_string = calloc(1, sizeof(char));
    size_t len = 0;

    for (size_t i = 0; i < special->nb_args; i++)
    {
        char *arg_string = special->args[i];
        size_t value_len = strlen(arg_string);

        if (i > 0)
        {
            full_string =
                realloc(full_string, sizeof(char) * (len + value_len + 2));
            strcat(full_string, " ");
            len += 1;
        }
        else
        {
            full_string =
                realloc(full_string, sizeof(char) * (len + value_len + 1));
        }

        strcat(full_string, arg_string);
        len += value_len;
    }

    full_string[len] = '\0';
    return full_string;
}

static char *get_random_str(void)
{
    char *rand_str = malloc(10);
    snprintf(rand_str, 10, "%d", rand() % 32767);
    return rand_str;
}

static char *get_ifs_str(void)
{
    char *ifs_env = getenv("IFS");
    if (ifs_env)
        return strdup(ifs_env);
    return strdup(" \t\n");
}

static char *get_uid_str(void)
{
    uid_t uid = getuid();
    struct passwd *p = getpwuid(uid);
    char *uid_str = malloc(10);
    snprintf(uid_str, 10, "%d", p->pw_uid);
    return uid_str;
}

static char *get_env_str(const char *var)
{
    char *variable = getenv(var);
    if (variable)
        return strdup(variable);
    return NULL;
}

static char *get_variable(char *var, struct special_vars *special)
{
    if (is_all_digits(var))
    {
        return strdup(special->args[atoi(var)]);
    }
    else if (strcmp(var, "?") == 0)
    {
        return get_last_value_str(special);
    }
    else if (strcmp(var, "#") == 0)
    {
        return get_nb_args_str(special);
    }
    else if (strcmp(var, "*") == 0 || strcmp(var, "@") == 0)
    {
        return join_all_args(special);
    }
    else if (strcmp(var, "RANDOM") == 0)
    {
        return get_random_str();
    }
    else if (strcmp(var, "IFS") == 0)
    {
        return get_ifs_str();
    }
    else if (strcmp(var, "UID") == 0)
    {
        return get_uid_str();
    }
    else
    {
        return get_env_str(var);
    }
}

void free_argv(char **argv)
{
    for (size_t i = 0; argv[i] != NULL; i++)
    {
        free(argv[i]);
    }
    free(argv);
}

static void check_argv_capacity(char ***argv, size_t *cap, size_t size)
{
    if (size >= *cap - 1)
    {
        *cap += 10;
        *argv = realloc(*argv, sizeof(char *) * (*cap));
    }
}

static char *get_base_value(const struct ast *child,
                            struct special_vars *special)
{
    if (child->type == VARIABLE)
        return get_variable(child->value, special);
    return strdup(child->value);
}

static void merge_next_value(char **base, const struct ast *child,
                             struct special_vars *special)
{
    char *val = child->value;
    int must_free = 0;

    if (child->type == VARIABLE || child->type == VARIABLE_LEFT)
    {
        val = get_variable(child->value, special);
        must_free = 1;
    }
    size_t new_len = strlen(*base) + strlen(val) + 2;
    *base = realloc(*base, new_len);
    strcat(*base, val);

    if (must_free)
        free(val);
}

static char *build_arg_for_child(const struct ast *node, size_t *i,
                                 struct special_vars *special)
{
    const struct ast *child = node->children[*i];
    enum token_type t = child->type;

    if (t == VARIABLE)
    {
        (*i)++;
        return get_variable(child->value, special);
    }
    else if (t == VARIABLE_RIGHT || t == FUNCTION_RIGHT)
    {
        char *base = (t == VARIABLE_RIGHT) ? get_variable(child->value, special)
                                           : strdup(child->value);

        (*i)++;
        const struct ast *next_child = node->children[*i];
        merge_next_value(&base, next_child, special);
        (*i)++;
        return base;
    }
    else if ((*i) < node->nb_children - 1)
    {
        const struct ast *next_child = node->children[*i + 1];
        if (next_child->type == VARIABLE_LEFT
            || next_child->type == FUNCTION_LEFT)
        {
            char *base = get_base_value(child, special);
            (*i)++;
            merge_next_value(&base, next_child, special);
            (*i)++;
            return base;
        }
    }

    (*i)++;
    if (child == NULL || child->value == NULL)
    {
        return NULL;
    }
    return strdup(child->value);
}

static char **set_exec_argv(struct ast *node, struct special_vars *special)
{
    size_t cap = 10;
    size_t size = 0;
    char **argv = calloc(cap, sizeof(char *));
    argv[size++] = strdup(node->value);

    size_t i = 0;
    while (i < node->nb_children)
    {
        check_argv_capacity(&argv, &cap, size);
        argv[size] = build_arg_for_child(node, &i, special);
        size++;
        argv[size] = NULL;
    }
    return argv;
}

static int exec(struct ast *node, struct special_vars *special,
                enum exit_status *status)
{
    char *val = node->value;
    if (node->type == VARIABLE)
    {
        val = getenv(val);
        if (!val)
        {
            int res = 0;
            for (size_t i = 0; i < node->nb_children; i++)
            {
                res = eval_ast(node->children[0], special, status);
            }
            return res;
        }
    }

    char **argv = set_exec_argv(node, special);
    int child_pid = fork();
    if (child_pid == 0)
    {
        int code = execvp(node->value, argv);
        if (code == -1)
        {
            free_argv(argv);
            fprintf(stderr, "%s unknow command\n", node->value);
            exit(127);
        }
    }
    else if (child_pid > 0)
    {
        int status;
        if (waitpid(child_pid, &status, 0) == -1)
        {
            free_argv(argv);
            return 127;
        }
        if (WIFEXITED(status))
        {
            free_argv(argv);
            return WEXITSTATUS(status);
        }
    }
    free_argv(argv);
    return 127;
}

static char *resolve_child_string(struct ast *A, struct special_vars *special,
                                  size_t i)
{
    char *var1 = strdup(A->children[i]->value);
    if (A->children[i]->type == VARIABLE
        || A->children[i]->type == VARIABLE_LEFT
        || A->children[i]->type == VARIABLE_RIGHT)
    {
        if (A->children[i]->value[0] == '\0')
        {
            free(var1);
            return strdup("$");
        }
        else
        {
            char *var2 = get_variable(A->children[i]->value, special);
            free(var1);

            if (!var2)
            {
                A->children[i]->type = VARIABLE_EMPTY;
                return strdup("");
            }

            return var2;
        }
    }
    return var1;
}

static int needs_space(struct ast *A, size_t i)
{
    if (i == 0)
        return 0;

    if ((A->children[i]->type != FUNCTION_LEFT
         && A->children[i - 1]->type != FUNCTION_RIGHT
         && A->children[i - 1]->type != VARIABLE_RIGHT
         && A->children[i]->type != VARIABLE_LEFT
         && A->children[i - 1]->type != VARIABLE_EMPTY
         && A->children[i]->type != FUNCTION_MIDDLE
         && A->children[i - 1]->type != FUNCTION_MIDDLE)
        || (i > 1 && (A->children[i - 1]->type == VARIABLE_EMPTY)))
    {
        return 1;
    }

    return 0;
}

static char *expand_full_string(char *full_string, size_t *p_len,
                                const char *child_str, int add_space)
{
    size_t value_len = strlen(child_str);
    size_t new_size = (*p_len) + value_len + 1;
    if (add_space)
        new_size += 1;

    char *tmp = realloc(full_string, new_size * sizeof(char));
    if (!tmp)
        return NULL;

    full_string = tmp;

    if (add_space)
    {
        strcat(full_string, " ");
        (*p_len) += 1;
    }

    strcat(full_string, child_str);
    (*p_len) += value_len;

    return full_string;
}

static char *build_echo_string(struct ast *A, struct special_vars *special)
{
    char *full_string = calloc(1, sizeof(char));
    if (!full_string)
        return NULL;

    size_t len = 0;

    for (size_t i = 0; i < A->nb_children; i++)
    {
        char *child_string = resolve_child_string(A, special, i);
        if (!child_string)
        {
            free(full_string);
            return NULL;
        }

        int add_space = needs_space(A, i);

        full_string =
            expand_full_string(full_string, &len, child_string, add_space);

        free(child_string);

        if (!full_string)
            return NULL;
    }

    full_string[len] = '\0';
    return full_string;
}

static int handle_variable(struct ast *A, struct special_vars *special,
                           enum exit_status *status, char **pval)
{
    if (A->type != VARIABLE)
        return -1;

    if (!strcmp(A->value, ""))
    {
        perror("unknow command $");
        return 127;
    }
    char *env_val = getenv(A->value);
    if (!env_val)
    {
        int res = 0;
        for (size_t i = 0; i < A->nb_children; i++)
            res = eval_ast(A->children[i], special, status);
        return res;
    }
    *pval = env_val;
    return -1;
}

static int handle_builtin_or_exec(const char *val, struct ast *A,
                                  struct special_vars *special,
                                  enum exit_status *status)
{
    if (!strcmp(val, "true"))
        return true();
    else if (!strcmp(val, "false"))
        return false();
    else if (!strcmp(val, "echo"))
    {
        char *full_string = build_echo_string(A, special);
        int n = echo(full_string);
        free(full_string);
        return n;
    }
    else if (!strcmp(val, "exit"))
    {
        *status = EXIT;
        if (A->nb_children == 0)
            return 0;
        if (A->children[0]->type == VARIABLE)
            return atoi(getenv(A->children[0]->value));
        return atoi(A->children[0]->value);
    }
    else if (!strcmp(val, "cd"))
    {
        if (A->nb_children == 0)
        {
            cd("");
        }
        else if (A->nb_children == 1)
        {
            cd(A->children[0]->value);
        }
        else
        {
            fprintf(stderr, "cd too many args");
            return 1;
        }
        return -2;
    }
    else if (!strcmp(val, "."))
    {
        return dot(A->children[0]->value, special);
    }
    else if (!strcmp(val, "export"))
    {
        return 0; // export(A->children[0]->value);
    }
    else if (!strcmp(val, "unset"))
    {
        return 0; // unset(A);
    }
    else
    {
        fflush(stdout);
        return exec(A, special, status);
    }
}

static int eval_func(struct ast *A, struct special_vars *special,
                     enum exit_status *status)
{
    char *val = A->value;
    int var_result = handle_variable(A, special, status, &val);
    if (var_result != -1)
    {
        return var_result;
    }

    int ret = handle_builtin_or_exec(val, A, special, status);
    if (ret == -2)
    {
        special->last_value = 0;
        return 0;
    }
    return ret;
}

int get_default_io(char *redir)
{
    if (strcmp(redir, "<") == 0 || strcmp(redir, "<&") == 0
        || strcmp(redir, "<>") == 0)
    {
        return STDIN_FILENO;
    }
    else
    {
        return STDOUT_FILENO;
    }
}

struct ast *find_function(struct ast *node)
{
    if (!node)
        return NULL;

    if (node->type == FUNCTION)
        return node;

    for (size_t i = 0; i < node->nb_children; i++)
    {
        struct ast *res = find_function(node->children[i]);
        if (res)
        {
            return res;
        }
    }
    return NULL;
}

static void parse_children(struct ast *node, struct ast **redirection,
                           struct ast **io_number, struct ast **child)
{
    for (size_t i = 0; i < node->nb_children; i++)
    {
        switch (node->children[i]->type)
        {
        case REDIRECTION:
            *redirection = node->children[i];
            break;
        case IONUMBER:
            *io_number = node->children[i];
            break;
        default:
            *child = node->children[i];
            break;
        }
    }
}

static int handle_filem_child(struct ast *child)
{
    if (child && child->type == FILEM)
    {
        int result = eval_redir(child);
        return result;
    }
    return 0;
}

static int get_io_value(struct ast *io_number, struct ast *redirection)
{
    if (io_number)
        return atoi(io_number->value);
    return get_default_io(redirection->value);
}

static int handle_fd_aggregation(const char *file, int io)
{
    int target_fd = atoi(file);
    if (dup2(target_fd, io) == -1)
    {
        perror("dup fail");
        return 2;
    }
    return 0;
}

static int open_redir_fd(const char *redir_value, const char *file)
{
    if (strcmp(redir_value, ">") == 0 || strcmp(redir_value, ">|") == 0)
        return open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    else if (strcmp(redir_value, ">>") == 0)
        return open(file, O_WRONLY | O_CREAT | O_APPEND, 0644);
    else if (strcmp(redir_value, "<") == 0)
        return open(file, O_RDONLY);
    else if (strcmp(redir_value, "<>") == 0)
        return open(file, O_RDWR | O_CREAT, 0644);

    return -2;
}

static int is_io_good(int io, char *redir)
{
    if ((io == 1 || io == 2) && redir[0] == '<')
    {
        return 0;
    }
    return 1;
}

int eval_redir(struct ast *node)
{
    struct ast *redirection = NULL;
    struct ast *io_number = NULL;
    struct ast *child = NULL;
    parse_children(node, &redirection, &io_number, &child);

    int result = handle_filem_child(child);
    if (result != 0)
        return result;

    int io = get_io_value(io_number, redirection);

    if (strcmp(redirection->value, ">&") == 0
        || strcmp(redirection->value, "<&") == 0)
        return handle_fd_aggregation(node->value, io);

    int fd = open_redir_fd(redirection->value, node->value);

    if (fd == -2)
    {
        perror("Invalid redirection");
        return 2;
    }

    if (fd == -1)
    {
        perror("open fail");
        return 2;
    }

    if (!is_io_good(io, redirection->value))
    {
        perror("io number no good");
        return 1;
    }

    if (dup2(fd, io) == -1)
    {
        perror("dup fail");
        close(fd);
        return 2;
    }

    close(fd);
    return 0;
}

static int eval_filem(struct ast *A, struct special_vars *special,
                      enum exit_status *status)
{
    int result = eval_redir(A);
    if (result != 0)
        return result;

    struct ast *func = find_function(A);
    if (func)
    {
        if (result == 1)
        {
            eval_ast(func, special, status);
            return 1;
        }
        return eval_ast(func, special, status);
    }

    return 0;
}

static int exec_pipe(struct ast *left, struct ast *right,
                     struct special_vars *special, enum exit_status *status)
{
    int pfd[2];
    pid_t pidl;
    pid_t pidr;

    if (pipe(pfd) == -1)
    {
        return 1;
    }

    pidl = fork();
    if (pidl == -1)
    {
        return 1;
    }
    if (pidl == 0)
    {
        close(pfd[0]);
        dup2(pfd[1], STDOUT_FILENO);
        close(pfd[1]);
        // execvp(argv_left[0], argv_left);
        exit(eval_ast(left, special, status));
        // exit(1);
    }

    pidr = fork();
    if (pidr == -1)
    {
        return 1;
    }
    if (pidr == 0)
    {
        close(pfd[1]);
        dup2(pfd[0], STDIN_FILENO);
        close(pfd[0]);
        // execvp(argv_right[0], argv_right);
        exit(eval_ast(right, special, status));
        // exit(1);
    }

    close(pfd[0]);
    close(pfd[1]);

    int sl;
    int sr;
    waitpid(pidl, &sl, 0);
    waitpid(pidr, &sr, 0);

    return WEXITSTATUS(sr);
}

static int eval_list(struct ast *A, struct special_vars *special,
                     enum exit_status *status)
{
    int res = 0;
    for (size_t i = 0; i < A->nb_children; i++)
    {
        res = eval_ast(A->children[i], special, status);
        if (*status == EXIT)
        {
            return res;
        }
        special->last_value = res;
    }
    return res;
}

static int eval_if(struct ast *A, struct special_vars *special,
                   enum exit_status *status)
{
    int condition = eval_ast(A->children[0], special, status);
    if (*status == EXIT)
    {
        return condition;
    }
    if (!condition)
    {
        int res = eval_ast(A->children[1], special, status);
        if (*status == EXIT)
        {
            return res;
        }
        special->last_value = res;
        return res;
    }
    else if (A->nb_children > 2)
    {
        int res = eval_ast(A->children[2], special, status);
        if (*status == EXIT)
        {
            return res;
        }
        special->last_value = res;
        return res;
    }
    else
    {
        special->last_value = 0;
        return 0;
    }
}

static int eval_pipe(struct ast *A, struct special_vars *special,
                     enum exit_status *status)
{
    if (A->nb_children == 1)
    {
        int res = eval_ast(A->children[0], special, status);
        special->last_value = res;
        return res;
    }
    else if (A->nb_children == 2)
    {
        return exec_pipe(A->children[0], A->children[1], special, status);
    }
    else if (A->nb_children == 3)
    {
        int res = exec_pipe(A->children[0], A->children[1], special, status);
        if (A->children[2]->type == PIPE)
        {
            return exec_pipe(A->children[1], A->children[2], special, status);
        }
        else
        {
            return res;
        }
    }
    return 2;
}

static int eval_negation(struct ast *A, struct special_vars *special,
                         enum exit_status *status)
{
    int res = eval_ast(A->children[0], special, status);
    if (*status == EXIT)
    {
        return res;
    }
    if (res == 0)
    {
        special->last_value = 1;
        return 1;
    }
    special->last_value = 0;
    return 0;
}

static int eval_while(struct ast *A, struct special_vars *special,
                      enum exit_status *status)
{
    int res = 0;
    int cond = eval_ast(A->children[0], special, status);
    if (*status == EXIT)
    {
        return res;
    }
    while (!cond)
    {
        res = eval_ast(A->children[1], special, status);
        if (*status == EXIT)
        {
            return res;
        }
        special->last_value = res;
        cond = eval_ast(A->children[0], special, status);
    }
    return res;
}

static int eval_until(struct ast *A, struct special_vars *special,
                      enum exit_status *status)
{
    int res = 0;
    int cond = eval_ast(A->children[0], special, status);
    if (*status == EXIT)
    {
        return cond;
    }
    while (cond != 0)
    {
        res = eval_ast(A->children[1], special, status);
        if (*status == EXIT)
        {
            return res;
        }
        special->last_value = res;
    }
    return res;
}

static int eval_equal(struct ast *A)
{
    struct ast *name = A->children[0];
    size_t i = 0;
    while (name->value[i] != '\0')
    {
        if (!((name->value[i] >= 'A' && name->value[i] <= 'Z')
              || (name->value[i] >= 'a' && name->value[i] <= 'z')
              || (name->value[i] >= '0' && name->value[i] <= '9')
              || (name->value[i] == '+' || name->value[i] == '_')))
        {
            perror("oui");
            return 127;
        }
        i++;
    }
    struct ast *value = A->children[1];
    char *val = value->value;
    if (value->type == VARIABLE)
    {
        val = getenv(val);
        if (val == NULL)
        {
            val = "";
        }
    }
    setenv(name->value, val, 1);
    return 0;
}

static int eval_for(struct ast *A, struct special_vars *special,
                    enum exit_status *status)
{
    char *var_name = A->children[0]->children[0]->value;
    int def = eval_ast(A->children[0], special, status);
    special->last_value = def;
    struct ast *list = A->children[1];

    int res = 0;
    special->last_value = 0;
    for (size_t i = 0; i < list->nb_children; i++)
    {
        setenv(var_name, list->children[i]->value, 1);
        res = eval_ast(A->children[2], special, status);
        if (*status == EXIT)
        {
            return res;
        }
        special->last_value = res;
    }
    return res;
}

static inline int update_last_value(struct special_vars *special, int res)
{
    special->last_value = res;
    return res;
}

static int handle_or(struct ast *A, struct special_vars *special,
                     enum exit_status *status)
{
    int left = eval_ast(A->children[0], special, status);
    if (*status == EXIT || left == 0)
    {
        return left;
    }

    int right = eval_ast(A->children[1], special, status);
    special->last_value = right;
    return right;
}

static int handle_and(struct ast *A, struct special_vars *special,
                      enum exit_status *status)
{
    int left = eval_ast(A->children[0], special, status);
    if (*status == EXIT || left != 0)
    {
        return left;
    }

    int right = eval_ast(A->children[1], special, status);
    special->last_value = right;
    return right;
}

int eval_ast(struct ast *A, struct special_vars *special,
             enum exit_status *status)
{
    if (A == NULL)
        return update_last_value(special, 0);

    switch (A->type)
    {
    case AST_LIST:
        return update_last_value(special, eval_list(A, special, status));
    case IF:
    case ELIF:
        return update_last_value(special, eval_if(A, special, status));
    case ELSE: {
        int res = eval_ast(A->children[0], special, status);
        return update_last_value(special, res);
    }
    case FILEM:
        return eval_filem(A, special, status);
    case PIPE:
        return update_last_value(special, eval_pipe(A, special, status));
    case NEGATION:
        return update_last_value(special, eval_negation(A, special, status));
    case WHILE:
        return update_last_value(special, eval_while(A, special, status));
    case UNTIL:
        return update_last_value(special, eval_until(A, special, status));
    case OR:
        return handle_or(A, special, status);
    case AND:
        return handle_and(A, special, status);

    case EQUAL:
        return update_last_value(special, eval_equal(A));
    case FOR:
        return update_last_value(special, eval_for(A, special, status));

    default:
        return update_last_value(special, eval_func(A, special, status));
    }
}
