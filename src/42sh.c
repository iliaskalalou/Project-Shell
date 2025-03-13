#define _POSIX_C_SOURCE 200809L

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast/ast.h"
#include "eval/eval.h"
#include "lexer.h"
#include "parser.h"

/*
const char *token_type_to_string(enum token_type type)
{
    switch (type)
    {
    case FUNCTION:
        return "FUNCTION";
    case EOL:
        return "EOL";
    case EOFF:
        return "EOFF";
    case IF:
        return "IF";
    case THEN:
        return "THEN";
    case ELIF:
        return "ELIF";
    case ELSE:
        return "ELSE";
    case FI:
        return "FI";
    case SINGLE_QUOTE:
        return "SINGLE_QUOTE";
    case ERREUR:
        return "ERREUR";
    case AST_LIST:
        return "AST_LIST";
    case NEGATION:
        return "NEGATION";
    case FUNCTION_LEFT:
        return "FUNCTION_LEFT";
    case FUNCTION_RIGHT:
        return "FUNCTION_RIGHT";
    case EXEC:
        return "EXEC";
    case IONUMBER:
        return "IONUMBER";
    case VARIABLE:
        return "VARIABLE";
    case AND:
        return "AND";
    case OR:
        return "OR";
    case VARIABLE_SUPP:
        return "VARIABLE_SUPP";
    case EQUAL:
        return "EQUAL";
    default:
        return "UNKNOWN";
    }
}

void print_ast(const struct ast *node, int depth)
{
    if (!node)
    {
        return;
    }

    for (int i = 0; i < depth; ++i)
    {
        printf("  ");
    }

    printf("Type: %s", token_type_to_string(node->type));
    if (node->value)
    {
        printf(", Value: %s", node->value);
    }
    printf("\n");

    for (size_t i = 0; i < node->nb_children; ++i)
    {
        print_ast(node->children[i], depth + 1);
    }
}
*/

static char *read_file(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
        fclose(file);
        return NULL;
    }

    long file_size = ftell(file);
    if (file_size < 0)
    {
        fclose(file);
        return NULL;
    }

    rewind(file);

    char *content = malloc(file_size + 1);
    if (!content)
    {
        fclose(file);
        return NULL;
    }

    long read_size = fread(content, 1, file_size, file);
    if (read_size != file_size)
    {
        free(content);
        fclose(file);
        return NULL;
    }

    content[file_size] = '\0';
    fclose(file);

    return content;
}

static char *read_stdin(void)
{
    size_t buffer_size = 1024;
    size_t total_size = 0;
    char *content = malloc(buffer_size);
    if (!content)
    {
        return NULL;
    }

    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), stdin)) > 0)
    {
        if (total_size + bytes_read >= buffer_size)
        {
            buffer_size *= 2;
            char *new_content = realloc(content, buffer_size);
            if (!new_content)
            {
                free(content);
                return NULL;
            }
            content = new_content;
        }
        memcpy(content + total_size, buffer, bytes_read);
        total_size += bytes_read;
    }

    if (ferror(stdin))
    {
        free(content);
        return NULL;
    }

    content[total_size] = '\0';
    return content;
}

struct special_vars *init_specials(const char *command)
{
    struct special_vars *vars = malloc(sizeof(struct special_vars));

    vars->last_value = 0;
    vars->args = NULL;

    char *copy = strdup(command);

    int size = 0;
    char *arg = strtok(copy, " ");
    while (arg)
    {
        char **splitted = realloc(vars->args, (size + 1) * sizeof(char *));
        vars->args = splitted;

        vars->args[size] = strdup(arg);

        size++;
        arg = strtok(NULL, " ");
    }

    char **splitted = realloc(vars->args, (size + 1) * sizeof(char *));
    vars->args = splitted;
    vars->args[size] = NULL;
    vars->nb_args = size;

    free(copy);
    return vars;
}

void free_specials(struct special_vars *vars)
{
    for (size_t i = 0; i < vars->nb_args; i++)
    {
        free(vars->args[i]);
    }
    free(vars->args);
    free(vars);
}

static int exec_arg(int argc, char **argv)
{
    int res = 0;
    if (argc == 3)
    {
        // Appeler avec argv[2]
        struct lexer *lex = lexer_new(argv[2]);
        enum parser_status status = PARSER_OK;
        struct ast *ast = parse(&status, lex);
        if (status == PARSER_UNEXPECTED_TOKEN)
        {
            ast_free(ast);
            lexer_free(lex);
            perror("Invalid expression");
            return 2;
        }
        //             print_ast(ast, 0);
        lexer_free(lex);
        struct special_vars *vars = init_specials(argv[2]);
        enum exit_status status_eval = RUNNING;
        res = eval_ast(ast, vars, &status_eval);
        free_specials(vars);
        ast_free(ast);
    }
    else
    {
        perror("Missing expression");
        return 1;
    }
    return res;
}

static int exec_file(char **argv)
{
    char *file_string = read_file(argv[1]);
    if (!file_string)
    {
        perror("Not a valid file");
        return 1;
    }
    //	printf("%s\n", file_string);
    struct lexer *lex = lexer_new(file_string);
    enum parser_status status = PARSER_OK;
    struct ast *ast = parse(&status, lex);
    if (status == PARSER_UNEXPECTED_TOKEN)
    {
        free(file_string);
        ast_free(ast);
        lexer_free(lex);
        perror("Invalid expression");
        return 2;
    }
    lexer_free(lex);
    // print_ast(ast, 0);
    struct special_vars *vars = init_specials(file_string);
    enum exit_status status_eval = RUNNING;
    int res = eval_ast(ast, vars, &status_eval);
    free_specials(vars);
    ast_free(ast);
    free(file_string);
    return res;
}

int exec_stdin(void)
{
    char *stdin_string = read_stdin();
    struct lexer *lex = lexer_new(stdin_string);
    enum parser_status status = PARSER_OK;
    struct ast *ast = parse(&status, lex);
    if (status == PARSER_UNEXPECTED_TOKEN)
    {
        free(stdin_string);
        ast_free(ast);
        lexer_free(lex);
        perror("Invalid expression");
        return 2;
    }
    lexer_free(lex);
    struct special_vars *vars = init_specials(stdin_string);
    enum exit_status status_eval = RUNNING;
    int res = eval_ast(ast, vars, &status_eval);
    free_specials(vars);
    ast_free(ast);
    free(stdin_string);
    return res;
}

int main(int argc, char **argv)
{
    if (argc > 3)
    {
        perror("Too much arguments");
        return 1;
    }
    int res = 0;
    if (argc == 1)
    {
        res = exec_stdin();
    }
    else
    {
        if (strcmp(argv[1], "-c") == 0)
        {
            res = exec_arg(argc, argv);
        }
        else
        {
            res = exec_file(argv);
        }
    }
    return res;
}
