#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ast/ast.h"
#include "eval.h"
#include "lexer.h"
#include "parser.h"

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

static int exec_file(char *file, struct special_vars *special)
{
    char *file_string = read_file(file);
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
    enum exit_status status_eval = RUNNING;
    int res = eval_ast(ast, special, &status_eval);
    ast_free(ast);
    free(file_string);
    return res;
}

int has_slash(char *file)
{
    while (*file)
    {
        if (*file == '/')
        {
            return 1;
        }
        file++;
    }
    return 0;
}

int dot(char *file, struct special_vars *special)
{
    if (has_slash(file))
    {
        return exec_file(file, special);
    }
    int res = 1;
    char *path = getenv("PATH");
    char *tok = strtok(path, ":");
    int was_exec = 0;
    DIR *d;
    struct dirent *dir;

    while (tok)
    {
        d = opendir(tok);
        if (d)
        {
            while ((dir = readdir(d)) != NULL)
            {
                if (strcmp(file, dir->d_name) == 0)
                {
                    if (tok[strlen(tok) - 1] != '/')
                    {
                        strcat(tok, "/");
                    }
                    strcat(tok, file);
                    res = exec_file(tok, special);
                    was_exec = 1;
                    break;
                }
            }
            closedir(d);
        }
        tok = strtok(NULL, ":");
    }
    if (!was_exec)
    {
        exec_file(file, special);
    }
    return res;
}
