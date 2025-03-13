#include "lexer.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"

// TODO

// faire en sorte que des que y'a un echo tous se qui est entre l'echo et le
// prochain EOL
//  soit considerer comme un string

/**
 * \brief Creates a new lexer given an input string.
 */
struct lexer *lexer_new(char *input)
{
    if (input == NULL)
    {
        return NULL;
    }
    struct lexer *lex = calloc(1, sizeof(struct lexer));
    lex->current_tok = calloc(1, sizeof(struct token));
    lex->input = input;
    lex->pos = 0;
    lex->current_tok->type = EOF;
    lex->current_tok->value = 0;
    lex->arg = 0;
    lex->arg2 = 0;
    lex->escap = 0;
    lex->arg_for = 0;
    lex->taille = 0;
    lex->virg_scape = 0;
    return lex;
}

/**
 ** \brief Frees the given lexer, but not its input.
 */
void lexer_free(struct lexer *lexer)
{
    token_free(lexer->current_tok);
    free(lexer);
}

/**
 * \brief Returns a token from the input string.

 * This function goes through the input string character by character and
 * builds a token. lexer_peek and lexer_pop should call it. If the input is
 * invalid, you must print something on stderr and return the appropriate token.
 */

static char *determined_type_part1(struct lexer *lexer, char *res, size_t *i,
                                   char *c)
{
    if (*c == 39 || *c == 34) // '
    {
        if (*c == 39)
        {
            res[*i] = 39;
        }
        else
        {
            res[*i] = 34;
        }
        (*i)++;
        *c = lexer->input[lexer->pos + *i];
        res[*i] = '\0';
        return res;
    }
    return NULL;
}

static char *determined_type_part2(struct lexer *lexer, char *res, size_t *i,
                                   char *c)
{
    if (*c == ';')
    {
        res[*i] = ';';
        (*i)++;
        *c = lexer->input[lexer->pos + *i];
        res[*i] = '\0';
        return res;
    }
    return NULL;
}

static char *determined_type_part3(struct lexer *lexer, char *res, size_t *i,
                                   char *c)
{
    if (*c == '|')
    {
        res[*i] = '|';
        (*i)++;
        *c = lexer->input[lexer->pos + *i];
        if (*c == '|')
        {
            res[*i] = '|';
            (*i)++;
            *c = lexer->input[lexer->pos + *i];
        }
        res[*i] = '\0';
        return res;
    }
    return NULL;
}

static char *determined_type_part4(struct lexer *lexer, char *res, size_t *i,
                                   char *c)
{
    if (*c == '&')
    {
        res[*i] = '&';
        (*i)++;
        *c = lexer->input[lexer->pos + *i];
        if (lexer->input[lexer->pos + *i] == '\\'
            && lexer->input[lexer->pos + *i + 1] == '\n'
            && lexer->input[lexer->pos + *i + 2] == '&')
        {
            res[*i] = '&';
            (*i) += 3;
            lexer->taille = 2;
            *c = lexer->input[lexer->pos + *i];
        }
        else if (*c == '&')
        {
            res[*i] = '&';
            (*i)++;
            *c = lexer->input[lexer->pos + *i];
        }
        res[*i] = '\0';
        return res;
    }
    return NULL;
}

static struct token *__switchh_part3(struct lexer *lexer, char *str);

static char *determined_type_part5(struct lexer *lexer, char *res, size_t *i,
                                   char *c)
{
    if (*c == '!'
        && (lexer->input[lexer->pos + 1] == ' '
            || lexer->input[lexer->pos + 1] == '\t'
            || lexer->input[lexer->pos + 1] == '\n'))
    {
        res[*i] = '!';
        (*i)++;
        *c = lexer->input[lexer->pos + *i];
        res[*i] = '\0';
        return res;
    }
    return NULL;
}

static char *determined_type_part6(struct lexer *lexer, char *res, size_t *i,
                                   char *c)
{
    if (*c == '`')
    {
        res[*i] = '`';
        (*i)++;
        *c = lexer->input[lexer->pos + *i];
        res[*i] = '\0';
        return res;
    }
    return NULL;
}

static char *determined_type_part7(struct lexer *lexer, char *res, size_t *i,
                                   char *c)
{
    if (*c == '=')
    {
        res[*i] = '=';
        (*i)++;
        *c = lexer->input[lexer->pos + *i];
        res[*i] = '\0';
        return res;
    }
    return NULL;
}

static char *determined_type_part8(struct lexer *lexer, char *res, size_t *i,
                                   char *c)
{
    if (*c == '$')
    {
        res[*i] = '$';
        (*i)++;
        *c = lexer->input[lexer->pos + *i];
        res[*i] = '\0';
        return res;
    }
    return NULL;
}

static char *determined_type_part80(struct lexer *lexer, char *res, char *c)
{
    if (*c == '<' || *c == '>')
    {
        res[0] = *c;
        *c = lexer->input[lexer->pos + 1];

        if (*c != '\0' && (*c == '>' || *c == '<' || *c == '&' || *c == '|'))
        {
            res[1] = *c;
            res[2] = '\0';
            return res;
        }
        res[1] = '\0';
        return res;
    }
    return NULL;
}

static char *determined_type_part9(struct lexer *lexer, char *res, size_t *i,
                                   char *c)
{
    size_t stop = 0;

    while (*c != '\0' && *c != '\n' && *c != ' ' && *c != '\t' && *c != '`'
           && *c != '>' && *c != '<')
    {
        /*if (*c != 92)
        {
            stop = 0;
        }*/
        if (*c == 92) // valeur du back slash
        {
            *c = lexer->input[lexer->pos + *i + 1];
            lexer->pos += 1;
            stop = 1;
            lexer->escap = 1;
            if (*c == '\n')
            {
                lexer->pos++;
                *c = lexer->input[lexer->pos + *i];
                continue;
            }
            if (*c == ';')
            {
                lexer->virg_scape = 1;
            }
            if (*c == '\0')
            {
                res[*i] = '\\';
                (*i)++;
            }
            else if (*c != '\\')
            {
                continue;
            }
        }
        else if (stop != 1)
        {
            if (*c == '|' || *c == '&' || *c == ';' || *c == 39 || *c == 34
                || *c == '=' || *c == '$' || *c == '>' || *c == '<')
            {
                break;
            }
        }
        stop = 0;
        res[*i] = *c;
        (*i)++;
        res[*i] = '\0';
        *c = lexer->input[lexer->pos + *i];
    }
    return res;
}

static char *determined_type(struct lexer *lexer)
{
    char c = lexer->input[lexer->pos];

    char *res = calloc(1, 100000);
    size_t i = 0;

    char *ret = determined_type_part1(lexer, res, &i, &c);
    if (ret)
    {
        return ret;
    }

    ret = determined_type_part80(lexer, res, &c);
    if (ret)
    {
        return ret;
    }

    ret = determined_type_part2(lexer, res, &i, &c);
    if (ret)
    {
        return ret;
    }

    ret = determined_type_part3(lexer, res, &i, &c);
    if (ret)
    {
        return ret;
    }

    ret = determined_type_part4(lexer, res, &i, &c);
    if (ret)
    {
        return ret;
    }

    ret = determined_type_part5(lexer, res, &i, &c);
    if (ret)
    {
        return ret;
    }

    ret = determined_type_part6(lexer, res, &i, &c);
    if (ret)
    {
        return ret;
    }

    ret = determined_type_part7(lexer, res, &i, &c);
    if (ret)
    {
        return ret;
    }

    ret = determined_type_part8(lexer, res, &i, &c);
    if (ret)
    {
        return ret;
    }

    ret = determined_type_part9(lexer, res, &i, &c);

    return ret;
}

/*static char *determined_type(struct lexer *lexer)
{
    char c = lexer->input[lexer->pos];

    char *res = calloc(1, 100000);
    size_t i = 0;

    if (c == 39 || c == 34) // '
    {
        if (c == 39)
        {
            res[i] = 39;
        }
        else
        {
            res[i] = 34;
        }
        i++;
        c = lexer->input[lexer->pos + i];
        res[i] = '\0';
        return res;
    }

    if (c == ';')
    {
        res[i] = ';';
        i++;
        c = lexer->input[lexer->pos + i];
        res[i] = '\0';
        return res;
    }

    if (c == '|')
    {
        res[i] = '|';
        i++;
        c = lexer->input[lexer->pos + i];
        if (c == '|')
        {
            res[i] = '|';
            i++;
            c = lexer->input[lexer->pos + i];
        }
        res[i] = '\0';
        return res;
    }
    if (c == '&')
    {
        res[i] = '&';
        i++;
        c = lexer->input[lexer->pos + i];
        if (c == '&')
        {
            res[i] = '&';
            i++;
            c = lexer->input[lexer->pos + i];
        }
        res[i] = '\0';
        return res;
    }

    if (c == '!')
    {
        res[i] = '!';
        i++;
        c = lexer->input[lexer->pos + i];
        res[i] = '\0';
        return res;
    }

    if (c == '`')
    {
        res[i] = '`';
        i++;
        c = lexer->input[lexer->pos + i];
        res[i] = '\0';
        return res;
    }

    if (c == '=')
    {
        res[i] = '=';
        i++;
        c = lexer->input[lexer->pos + i];
        res[i] = '\0';
        return res;
    }

    if (c == '$')
    {
        res[i] = '$';
        i++;
        c = lexer->input[lexer->pos + i];
        res[i] = '\0';
        return res;
    }

    size_t stop = 0;

    while (c != '\0' && c != '\n' && c != ' ' && c != ';' && c != 39*
           && c != '\t' && c != '`' && c != '|' && c != '!' && c != '&')
    {

        if (c == 92) // valeur du back slash
        {
            c = lexer->input[lexer->pos + i + 1];
            lexer->pos += 1;
            stop = 1;
            lexer->escap = 1;
            if (c == '\n')
            {
                lexer->pos++;
                c = lexer->input[lexer->pos + i];
            }
            continue;
        }
        else if (stop != 1)
        {
            if (c == '|' || c == '&' || c == ';' || c == 39 || c == 34
                || c == '=' || c == '$')
            {
                break;
            }
        }
        stop = 0;
        res[i] = c;
        i++;
        res[i] = '\0';
        c = lexer->input[lexer->pos + i];
    }
    return res;
}*/

static void clear(char *str)
{
    for (size_t i = 0; i < 100000; i++)
    {
        str[i] = '\0';
    }
}

static struct token *__lexer_fin(void)
{
    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = EOFF;
    tok->value = NULL;
    // struct token tok = { .type = EOFF, .value = NULL };
    return tok;
}

static struct token *__lexer_nnn(struct lexer *lexer)
{
    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = EOL;
    tok->value = calloc(1, 100);
    tok->value[0] = 'n';
    tok->value[1] = '\0';
    char c = lexer->input[lexer->pos];
    while ((c == '\n' || c == ' ' || c == '\t') && c != '\0')
    {
        lexer->pos++;
        c = lexer->input[lexer->pos];
    }
    //        struct token tok = { .type = EOL, .value = NULL };
    return tok;
}

static struct token *__lexer_if(struct lexer *lexer, char *str)
{
    free(str);
    lexer->pos += 2;
    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = IF;
    tok->value = calloc(1, 100);
    tok->value[0] = '!';
    tok->value[1] = '\0';
    //       struct token tok = { .type = IF, .value = NULL };
    return tok;
}

static struct token *__lexer_else(struct lexer *lexer, char *str)
{
    free(str);
    lexer->pos += 4;

    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = ELSE;
    tok->value = calloc(1, 100);
    tok->value[0] = '!';
    tok->value[1] = '\0';
    // struct token tok = { .type = ELSE, .value = NULL };
    return tok;
}

static struct token *__lexer_elif(struct lexer *lexer, char *str)
{
    free(str);
    lexer->pos += 4;

    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = ELIF;
    tok->value = calloc(1, 100);
    tok->value[0] = '!';
    tok->value[1] = '\0';

    //        struct token tok = { .type = ELIF, .value = NULL };
    return tok;
}

static struct token *__lexer_then(struct lexer *lexer, char *str)
{
    free(str);
    lexer->pos += 4;

    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = THEN;
    tok->value = calloc(1, 100);
    tok->value[0] = '!';
    tok->value[1] = '\0';

    //       struct token tok = { .type = THEN, .value = NULL };
    return tok;
}

static struct token *__lexer_fi(struct lexer *lexer, char *str)
{
    free(str);
    lexer->pos += 2;

    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = FI;
    tok->value = calloc(1, 100);
    tok->value[0] = '!';
    tok->value[1] = '\0';

    //       struct token tok = { .type = FI, .value = NULL };
    return tok;
}

static struct token *__lexer_quote_part1(/*enum parser_status *status,*/
                                         struct lexer *lexer, char *str)
{
    lexer->pos += 1;

    free(str);
    if (lexer->input[lexer->pos] == '\0')
    {
        /*status = PARSER_UNEXPECTED_TOKEN;
            return NULL;
        struct token *tok = calloc(1, sizeof(struct token));
     tok->type = OR;
     tok->value = calloc(1,100000);
     tok->value[0] = '\0';
     return tok;*/
    }
    if (lexer->input[lexer->pos] == 39) // cas ou '' coller
    {
        lexer->pos += 1;
        /*if (lexer->input[lexer->pos] == '\0' && (lexer->pos - 2 != 0))
        {
            struct token *tok = calloc(1, sizeof(struct token));
            tok->type = EOFF;
                tok->value = calloc(1, 100000);
                tok->value[0] = '!';
                tok->value[1] = '\0';
            *status =  PARSER_UNEXPECTED_TOKEN;
        }*/
        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = FUNCTION;
        tok->value = calloc(1, 100000);
        tok->value[0] = '\0';
        return tok;
        // return lexer_next_token(status, lexer);
    }

    return NULL;
}

static size_t __lexer_quote_part2(enum parser_status *status,
                                  struct lexer *lexer, char **res_ptr,
                                  size_t *i_ptr)
{
    char *res = calloc(1, 100000);
    clear(res);
    size_t i = 0;

    size_t peux_etre_middle = 0;
    if (lexer->pos >= 2 && lexer->input[lexer->pos - 2] == '\'')
    {
        peux_etre_middle++;
    }

    while (lexer->input[lexer->pos] != 39
           && lexer->input[lexer->pos] != '\0') //   '
    {
        if (lexer->input[lexer->pos] == 92
            && (lexer->input[lexer->pos] == 39
                || lexer->input[lexer->pos + 1] == 34
                || lexer->input[lexer->pos + 1] == '\0'
                || lexer->input[lexer->pos + 1] == '\n'
                || lexer->input[lexer->pos + 1] == '\\'
                || lexer->input[lexer->pos + 1] == '$'
                || lexer->input[lexer->pos + 1] == '`'))
        {
            lexer->pos++;
            if (lexer->input[lexer->pos] == '\0')
            {
                *status = PARSER_UNEXPECTED_TOKEN;
                break;
            }
            res[i] = lexer->input[lexer->pos];
            i++;
            lexer->pos++;
        }
        else
        {
            res[i] = lexer->input[lexer->pos];
            i++;
            lexer->pos++;
        }
    }
    if (lexer->input[lexer->pos] == '\0')
    {
        *status = PARSER_UNEXPECTED_TOKEN;
        free(res);
        return 1;
    }
    if (lexer->input[lexer->pos + 2] == '\'')
    {
        if (peux_etre_middle == 1)
        {
            return 2;
        }
    }

    *res_ptr = res;
    *i_ptr = i;
    return 0;
}

static void __lexer_quote_part3(enum parser_status *status, struct lexer *lexer,
                                char *res, size_t *i)
{
    if (lexer->input[lexer->pos] == '\0'
        /*  || lexer->input[lexer->pos] == '\''*/)
    {
        *status = PARSER_UNEXPECTED_TOKEN;
        return;
    }
    if (lexer->input[lexer->pos + 1] == '\''
        && lexer->input[lexer->pos + 2] == '\0')
    {
        *status = PARSER_UNEXPECTED_TOKEN;
        return;
    }

    lexer->pos += 1;

    while (lexer->input[lexer->pos] != '\0')
    {
        if (lexer->input[lexer->pos] == 39)
        {
            lexer->pos++;
            while (lexer->input[lexer->pos] != 39
                   && lexer->input[lexer->pos] != '\0')
            {
                if (lexer->input[lexer->pos] == 92
                    && (lexer->input[lexer->pos + 1] == 39
                        || lexer->input[lexer->pos + 1] == 34
                        || lexer->input[lexer->pos + 1] == '\0'
                        || lexer->input[lexer->pos + 1] == '\n'
                        || lexer->input[lexer->pos + 1] == '\\'
                        || lexer->input[lexer->pos + 1] == '$'
                        || lexer->input[lexer->pos + 1] == '`'))
                {
                    lexer->pos++;
                    if (lexer->input[lexer->pos] == '\0')
                    {
                        *status = PARSER_UNEXPECTED_TOKEN;
                        break;
                    }
                    res[*i] = lexer->input[lexer->pos];
                    (*i)++;
                    lexer->pos++;
                }
                else
                {
                    res[*i] = lexer->input[lexer->pos];
                    (*i)++;
                    lexer->pos++;
                }
            }
            lexer->pos++;
        }
        else
        {
            break;
        }
    }
}

static struct token *__lexer_quote_part4(char *res, size_t mid)
{
    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = FUNCTION;
    if (mid == 2)
    {
        tok->type = FUNCTION_MIDDLE;
    }
    tok->value = res;
    tok->string = 1;
    //       struct token tok = { .type = SINGLE_QUOTE, .value = NULL };
    return tok;
}

static struct token *__lexer_quote(enum parser_status *status,
                                   struct lexer *lexer, char *str)
{
    struct token *tmp = __lexer_quote_part1(/*status,*/ lexer, str);
    if (tmp)
    {
        return tmp;
    }

    char *res = NULL;
    size_t i = 0;
    size_t middle = __lexer_quote_part2(status, lexer, &res, &i);

    __lexer_quote_part3(status, lexer, res, &i);

    return __lexer_quote_part4(res, middle);
}

/*
static struct token *__lexer_quote(enum parser_status *status,
                                   struct lexer *lexer, char *str)
{
    lexer->pos += 1;

    free(str);

    if (lexer->input[lexer->pos] == 39) // cas ou '' coller
    {
        lexer->pos += 1;
        return lexer_next_token(status, lexer);
    }

    char *res = calloc(1, 100000);
    clear(res);
    size_t i = 0;
    while (lexer->input[lexer->pos] != 39
           && lexer->input[lexer->pos] != '\0') //   '
    {
        if (lexer->input[lexer->pos] == 92
            && (lexer->input[lexer->pos] == 39
                || lexer->input[lexer->pos + 1] == 34
                || lexer->input[lexer->pos + 1] == '\0'
                || lexer->input[lexer->pos + 1] == '\n'
                || lexer->input[lexer->pos + 1] == '\\'
                || lexer->input[lexer->pos + 1] == '$'
                || lexer->input[lexer->pos + 1] == '`'))
        {
            lexer->pos++;
            if (lexer->input[lexer->pos] == '\0')
            {
                *status = PARSER_UNEXPECTED_TOKEN;
                break;
            }
            res[i] = lexer->input[lexer->pos];
            i++;
            lexer->pos++;
        }
        else
        {
            res[i] = lexer->input[lexer->pos];
            i++;
            lexer->pos++;
        }
    }

    if (lexer->input[lexer->pos] == '\0')
    {
        *status = PARSER_UNEXPECTED_TOKEN;
    }

    lexer->pos += 1;

    while (1)
    {
        if (lexer->input[lexer->pos] == 39)
        {
            lexer->pos++;
            while (lexer->input[lexer->pos] != 39)
            {
                if (lexer->input[lexer->pos] == 92
                    && (lexer->input[lexer->pos + 1] == 39
                        || lexer->input[lexer->pos + 1] == 34
                        || lexer->input[lexer->pos + 1] == '\0'
                        || lexer->input[lexer->pos + 1] == '\n'
                        || lexer->input[lexer->pos + 1] == '\\'
                        || lexer->input[lexer->pos + 1] == '$'
                        || lexer->input[lexer->pos + 1] == '`'))
                {
                    lexer->pos++;
                    if (lexer->input[lexer->pos] == '\0')
                    {
                        *status = PARSER_UNEXPECTED_TOKEN;
                        break;
                    }
                    res[i] = lexer->input[lexer->pos];
                    i++;
                    lexer->pos++;
                }
                else
                {
                    res[i] = lexer->input[lexer->pos];
                    i++;
                    lexer->pos++;
                }
            }
            lexer->pos++;
        }
        else
        {
            break;
        }
    }

    // lexer->pos += 1;

    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = FUNCTION;
    tok->value = res;
    tok->string = 1;
    return tok;
}*/

static struct token *__lexer_virgule(struct lexer *lexer, char *str)
{
    free(str);
    lexer->pos += 1;

    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = EOL;
    tok->value = calloc(1, 100);
    tok->value[0] = 'v';
    tok->value[1] = '\0';

    return tok;
}

static struct token *__lexer_pipe(struct lexer *lexer, char *str)
{
    free(str);
    lexer->pos += 1;

    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = PIPE;
    tok->value = calloc(1, 100);
    tok->value[0] = '|';
    tok->value[1] = '\0';

    //       struct token tok = { .type = EOL, .value = NULL };
    return tok;
}

static struct token *__lexer_negation(struct lexer *lexer, char *str)
{
    free(str);

    lexer->pos += 1;

    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = NEGATION;
    tok->value = calloc(1, 100);
    tok->value[0] = '!';
    tok->value[1] = '\0';

    //       struct token tok = { .type = EOL, .value = NULL };
    return tok;
}

static struct token *__lexer_while(struct lexer *lexer, char *str)
{
    free(str);
    lexer->pos += 5;

    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = WHILE;
    tok->value = calloc(1, 100);
    tok->value[0] = '!';
    tok->value[1] = '\0';

    //       struct token tok = { .type = EOL, .value = NULL };
    return tok;
}

static struct token *__lexer_until(struct lexer *lexer, char *str)
{
    free(str);
    lexer->pos += 5;

    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = UNTIL;
    tok->value = calloc(1, 100);
    tok->value[0] = '!';
    tok->value[1] = '\0';

    //       struct token tok = { .type = EOL, .value = NULL };
    return tok;
}

static struct token *__lexer_do(struct lexer *lexer, char *str)
{
    free(str);
    lexer->pos += 2;

    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = DO;
    tok->value = calloc(1, 100);
    tok->value[0] = '!';
    tok->value[1] = '\0';

    //       struct token tok = { .type = EOL, .value = NULL };
    return tok;
}

static struct token *__lexer_done(struct lexer *lexer, char *str)
{
    free(str);
    lexer->pos += 4;

    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = DONE;
    tok->value = calloc(1, 100);
    tok->value[0] = '!';
    tok->value[1] = '\0';

    //       struct token tok = { .type = EOL, .value = NULL };
    return tok;
}

static struct token *__lexer_and(struct lexer *lexer, char *str)
{
    free(str);
    lexer->pos += 2;
    lexer->pos += lexer->taille;
    lexer->taille = 0;
    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = AND;
    tok->value = calloc(1, 100);
    tok->value[0] = '!';
    tok->value[1] = '\0';

    //       struct token tok = { .type = EOL, .value = NULL };
    return tok;
}

static struct token *__lexer_or(struct lexer *lexer, char *str)
{
    free(str);
    lexer->pos += 2;
    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = OR;
    tok->value = calloc(1, 100);
    tok->value[0] = '!';
    tok->value[1] = '\0';

    //       struct token tok = { .type = EOL, .value = NULL };
    return tok;
}

static struct token *__lexer_quote_chelou(struct lexer *lexer, char *str)
{
    lexer->pos += 1;

    free(str);
    char *res = calloc(1, 100000);
    clear(res);
    size_t i = 0;
    while (lexer->input[lexer->pos] != '`') //
    {
        res[i] = lexer->input[lexer->pos];
        i++;
        lexer->pos++;
    }

    lexer->pos += 1;

    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = EXEC;
    tok->value = res;
    tok->string = 1;

    //       struct token tok = { .type = SINGLE_QUOTE, .value = NULL };
    return tok;
}

/*
struct double_quote_context
{
    char *res;
    size_t i;
    size_t var;
    char c;
};

static struct token *
__lexer_double_quote_part1(enum parser_status *status, struct lexer *lexer,
                           char *str, struct double_quote_context *ctx)
{
    lexer->pos += 1;

    free(str);

    if (lexer->input[lexer->pos] == 34) // cas ou '' coller
    {
        lexer->pos += 1;
        return lexer_next_token(status, lexer);
    }

    (void)ctx;
    return NULL;
}

static void __lexer_double_quote_part2(enum parser_status *status,
                                       struct lexer *lexer,
                                       struct double_quote_context *ctx)
{
    char *res = calloc(1, 100000);
    clear(res);
    size_t i = 0;
    size_t var = 0;
    char c = lexer->input[lexer->pos + 1];
    if (lexer->input[lexer->pos] == '$'
        && ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
            || (c >= '0' && c <= '9') || (c == '_' || c == '+')))
    {
        var = 1;
        lexer->pos++;
    }

    ctx->res = res;
    ctx->i = i;
    ctx->var = var;
    ctx->c = c;

    (void)status;
}

static void __lexer_double_quote_part3(enum parser_status *status,
                                       struct lexer *lexer,
                                       struct double_quote_context *ctx)
{
    char *res = ctx->res;
    size_t i = ctx->i;
    size_t var = ctx->var;
    char c = ctx->c;

    while (lexer->input[lexer->pos] != 34
           && lexer->input[lexer->pos] != '\0') //   '
    {
        if (lexer->input[lexer->pos] == 92
            && (lexer->input[lexer->pos + 1] == 39
                || lexer->input[lexer->pos + 1] == 34
                || lexer->input[lexer->pos + 1] == '\0'
                || lexer->input[lexer->pos + 1] == '\n'
                || lexer->input[lexer->pos + 1] == '\\'
                || lexer->input[lexer->pos + 1] == '$'
                || lexer->input[lexer->pos + 1] == '`'))
        {
            lexer->pos++;
            if (lexer->input[lexer->pos] == '\0')
            {
                *status = PARSER_UNEXPECTED_TOKEN;
                break;
            }
            res[i] = lexer->input[lexer->pos];
            i++;
            lexer->pos++;
        }
        else
        {
            res[i] = lexer->input[lexer->pos];
            i++;
            lexer->pos++;
        }
    }

    ctx->res = res;
    ctx->i = i;
    ctx->var = var;
    ctx->c = c;
}

static struct token *
__lexer_double_quote_part4(enum parser_status *status, struct lexer *lexer,
                           struct double_quote_context *ctx)
{
    char *res = ctx->res;
    size_t var = ctx->var;

    if (lexer->input[lexer->pos] == '\0')
    {
        *status = PARSER_UNEXPECTED_TOKEN;
        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = FUNCTION;
        tok->value = res;
        if (var == 1)
        {
            tok->type = VARIABLE;
        }
        return tok;
    }

    return NULL;
}

static struct token *
__lexer_double_quote_part5(struct lexer *lexer,
                           struct double_quote_context *ctx)
{
    char *res = ctx->res;
    size_t var = ctx->var;

    if (lexer->input[lexer->pos] == '\"')
    {
        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = FUNCTION;
        tok->value = res;
        lexer->pos++;
        if (lexer->input[lexer->pos] == '\"' || lexer->input[lexer->pos] == '\''
            || lexer->input[lexer->pos] == '$')
        {
            tok->type = FUNCTION_RIGHT;
        }
        tok->string = 1;
        if (var == 1)
        {
            tok->type = VARIABLE;
        }
        return tok;
    }

    return NULL;
}

static void __lexer_double_quote_part6(enum parser_status *status,
                                       struct lexer *lexer,
                                       struct double_quote_context *ctx)
{
    char *res = ctx->res;
    size_t i = ctx->i;

    lexer->pos += 1;

    while (1)
    {
        if (lexer->input[lexer->pos] == '\0')
        {
            break;
        }
        if (lexer->input[lexer->pos] == 34)
        {
            lexer->pos++;
            while (lexer->input[lexer->pos] == '\0'
                   && lexer->input[lexer->pos] != 34)
            {
                if (lexer->input[lexer->pos] == 92
                    && (lexer->input[lexer->pos + 1] == 39
                        || lexer->input[lexer->pos + 1] == 34
                        || lexer->input[lexer->pos + 1] == '\0'
                        || lexer->input[lexer->pos + 1] == '\n'
                        || lexer->input[lexer->pos + 1] == '\\'
                        || lexer->input[lexer->pos + 1] == '$'
                        || lexer->input[lexer->pos + 1] == '`'))
                {
                    lexer->pos++;
                    if (lexer->input[lexer->pos] == '\0')
                    {
                        *status = PARSER_UNEXPECTED_TOKEN;
                        break;
                    }
                    res[i] = lexer->input[lexer->pos];
                    i++;
                    lexer->pos++;
                }
                else
                {
                    res[i] = lexer->input[lexer->pos];
                    i++;
                    lexer->pos++;
                }
            }
            lexer->pos++;
        }
        else
        {
            break;
        }
    }

    ctx->i = i;
}

static struct token *
__lexer_double_quote_part7(struct double_quote_context *ctx)
{
    char *res = ctx->res;
    size_t var = ctx->var;

    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = FUNCTION;
    tok->value = res;
    tok->string = 1;
    if (var == 1)
    {
        tok->type = VARIABLE;
    }

    return tok;
}

static struct token *__lexer_double_quote(enum parser_status *status,
                                          struct lexer *lexer, char *str)
{
    struct double_quote_context ctx;
    ctx.res = NULL;
    ctx.i = 0;
    ctx.var = 0;
    ctx.c = 0;

    struct token *tok = __lexer_double_quote_part1(status, lexer, str, &ctx);
    if (tok)
        return tok;

    __lexer_double_quote_part2(status, lexer, &ctx);

    __lexer_double_quote_part3(status, lexer, &ctx);

    tok = __lexer_double_quote_part4(status, lexer, &ctx);
    if (tok)
    {
        return tok;
    }

    tok = __lexer_double_quote_part5(lexer, &ctx);
    if (tok)
    {
        return tok;
    }

    __lexer_double_quote_part6(status, lexer, &ctx);

    return __lexer_double_quote_part7(&ctx);
}*/

struct token *lexer_next_token(enum parser_status *status, struct lexer *lexer);

struct double_quote_ctx
{
    char *res;
    size_t i;
    size_t var;
    char c;
};

static struct token *__lexer_double_quote_partA(enum parser_status *status,
                                                struct lexer *lexer, char *str,
                                                struct double_quote_ctx *ctx)
{
    lexer->pos += 1;

    free(str);

    if (lexer->input[lexer->pos] == 34) // cas ou '' coller
    {
        lexer->pos += 1;
        return lexer_next_token(status, lexer);
    }

    char *res = calloc(1, 100000);
    clear(res);
    size_t i = 0;
    size_t var = 0;
    char c = lexer->input[lexer->pos + 1];

    if (lexer->input[lexer->pos] == '$'
        && ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
            || (c >= '0' && c <= '9') || (c == '_' || c == '+')))
    {
        var = 1;
        lexer->pos++;
    }

    ctx->res = res;
    ctx->i = i;
    ctx->var = var;
    ctx->c = c;

    return (struct token *)-1;
}

static struct token *__lexer_double_quote_partB(enum parser_status *status,
                                                struct lexer *lexer,
                                                struct double_quote_ctx *ctx)
{
    char *res = ctx->res;
    size_t i = ctx->i;

    while (lexer->input[lexer->pos] != 34
           && lexer->input[lexer->pos] != '\0') //   '
    {
        if (lexer->input[lexer->pos] == 92
            && ( // lexer->input[lexer->pos + 1] == 39
                lexer->input[lexer->pos + 1] == 34
                || lexer->input[lexer->pos + 1] == '\0'
                || lexer->input[lexer->pos + 1] == '\n'
                || lexer->input[lexer->pos + 1] == '\\'
                || lexer->input[lexer->pos + 1] == '$'
                || lexer->input[lexer->pos + 1] == '`'))
        {
            lexer->pos++;
            if (lexer->input[lexer->pos] == '\0')
            {
                *status = PARSER_UNEXPECTED_TOKEN;
                break;
            }
            res[i] = lexer->input[lexer->pos];
            i++;
            lexer->pos++;
        }
        else
        {
            res[i] = lexer->input[lexer->pos];
            i++;
            lexer->pos++;
        }
    }

    ctx->res = res;
    ctx->i = i;

    return (struct token *)-1;
}

static struct token *__lexer_double_quote_partC(enum parser_status *status,
                                                struct lexer *lexer,
                                                struct double_quote_ctx *ctx)
{
    char *res = ctx->res;
    size_t var = ctx->var;

    if (lexer->input[lexer->pos] == '\0')
    {
        *status = PARSER_UNEXPECTED_TOKEN;
        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = FUNCTION;
        tok->value = res;
        if (var == 1)
        {
            tok->type = VARIABLE;
        }
        return tok;
    }

    return (struct token *)-1;
}

static struct token *__lexer_double_quote_partD(struct lexer *lexer,
                                                struct double_quote_ctx *ctx)
{
    char *res = ctx->res;
    size_t var = ctx->var;
    size_t i = ctx->i; // juste au cas où

    if (lexer->input[lexer->pos] == '\"')
    {
        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = FUNCTION;
        tok->value = res;
        lexer->pos++;
        if (lexer->input[lexer->pos] == '\"' || lexer->input[lexer->pos] == '\''
            || lexer->input[lexer->pos] == '$')
        {
            tok->type = FUNCTION_RIGHT;
        }
        tok->string = 1;
        if (var == 1)
        {
            tok->type = VARIABLE;
        }
        return tok;
    }

    ctx->res = res;
    ctx->var = var;
    ctx->i = i;
    return (struct token *)-1;
}

static struct token *__lexer_double_quote_partE(enum parser_status *status,
                                                struct lexer *lexer,
                                                struct double_quote_ctx *ctx)
{
    char *res = ctx->res;
    size_t i = ctx->i;

    lexer->pos += 1;

    while (1)
    {
        if (lexer->input[lexer->pos] == '\0')
        {
            break;
        }
        if (lexer->input[lexer->pos] == 34)
        {
            lexer->pos++;
            while (lexer->input[lexer->pos] == '\0'
                   && lexer->input[lexer->pos] != 34)
            {
                if (lexer->input[lexer->pos] == 92
                    && (lexer->input[lexer->pos + 1] == 39
                        || lexer->input[lexer->pos + 1] == 34
                        || lexer->input[lexer->pos + 1] == '\0'
                        || lexer->input[lexer->pos + 1] == '\n'
                        || lexer->input[lexer->pos + 1] == '\\'
                        || lexer->input[lexer->pos + 1] == '$'
                        || lexer->input[lexer->pos + 1] == '`'))
                {
                    lexer->pos++;
                    if (lexer->input[lexer->pos] == '\0')
                    {
                        *status = PARSER_UNEXPECTED_TOKEN;
                        break;
                    }
                    res[i] = lexer->input[lexer->pos];
                    i++;
                    lexer->pos++;
                }
                else
                {
                    res[i] = lexer->input[lexer->pos];
                    i++;
                    lexer->pos++;
                }
            }
            lexer->pos++;
        }
        else
        {
            break;
        }
    }

    ctx->res = res;
    ctx->i = i;

    return (struct token *)-1;
}

static struct token *__lexer_double_quote_partF(enum parser_status *status,
                                                struct lexer *lexer,
                                                struct double_quote_ctx *ctx)
{
    char *res = ctx->res;
    size_t var = ctx->var;

    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = FUNCTION;
    tok->value = res;
    tok->string = 1;
    if (var == 1)
    {
        tok->type = VARIABLE;
    }
    return tok;
    (void)status;
    (void)lexer;
}

static struct token *__lexer_double_quote(enum parser_status *status,
                                          struct lexer *lexer, char *str)
{
    struct double_quote_ctx ctx;
    ctx.res = NULL;
    ctx.i = 0;
    ctx.var = 0;
    ctx.c = 0;

    struct token *ret = __lexer_double_quote_partA(status, lexer, str, &ctx);
    if (ret != (struct token *)-1)
    {
        return ret;
    }

    ret = __lexer_double_quote_partB(status, lexer, &ctx);
    if (ret != (struct token *)-1)
    {
        return ret;
    }

    ret = __lexer_double_quote_partC(status, lexer, &ctx);
    if (ret != (struct token *)-1)
    {
        return ret;
    }

    ret = __lexer_double_quote_partD(lexer, &ctx);
    if (ret != (struct token *)-1)
    {
        return ret;
    }

    ret = __lexer_double_quote_partE(status, lexer, &ctx);
    if (ret != (struct token *)-1)
    {
        return ret;
    }

    ret = __lexer_double_quote_partF(status, lexer, &ctx);
    return ret; // c'est le retour final
}

/*static struct token *__lexer_double_quote(enum parser_status *status,
                                          struct lexer *lexer, char *str)
{
    lexer->pos += 1;

    free(str);

    if (lexer->input[lexer->pos] == 34) // cas ou '' coller
    {
        lexer->pos += 1;
        return lexer_next_token(status, lexer);
    }

    char *res = calloc(1, 100000);
    clear(res);
    size_t i = 0;
    size_t var = 0;
    char c = lexer->input[lexer->pos + 1];
    if (lexer->input[lexer->pos] == '$'
        && ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
            || (c >= '0' && c <= '9') || (c == '_' || c == '+')))
    {
        var = 1;
        lexer->pos++;
    }
    while (lexer->input[lexer->pos] != 34
           && lexer->input[lexer->pos] != '\0') //   '
    {
        if (lexer->input[lexer->pos] == 92
            && ( // lexer->input[lexer->pos + 1] == 39
                lexer->input[lexer->pos + 1] == 34
                || lexer->input[lexer->pos + 1] == '\0'
                || lexer->input[lexer->pos + 1] == '\n'
                || lexer->input[lexer->pos + 1] == '\\'
                || lexer->input[lexer->pos + 1] == '$'
                || lexer->input[lexer->pos + 1] == '`'))
        {
            lexer->pos++;
            if (lexer->input[lexer->pos] == '\0')
            {
                *status = PARSER_UNEXPECTED_TOKEN;
                break;
            }
            res[i] = lexer->input[lexer->pos];
            i++;
            lexer->pos++;
        }
        else
        {
            res[i] = lexer->input[lexer->pos];
            i++;
            lexer->pos++;
        }
    }

    if (lexer->input[lexer->pos] == '\0')
    {
        *status = PARSER_UNEXPECTED_TOKEN;
        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = FUNCTION;
        tok->value = res;
        if (var == 1)
        {
            tok->type = VARIABLE;
        }
        return tok;
    }

    if (lexer->input[lexer->pos] == '\"')
    {
        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = FUNCTION;
        tok->value = res;
        lexer->pos++;
        if (lexer->input[lexer->pos] == '\"' || lexer->input[lexer->pos] == '\''
            || lexer->input[lexer->pos] == '$')
        {
            tok->type = FUNCTION_RIGHT;
        }
        tok->string = 1;
        if (var == 1)
        {
            tok->type = VARIABLE;
        }
        return tok;
    }

    lexer->pos += 1;

    while (1)
    {
        if (lexer->input[lexer->pos] == '\0')
        {
            break;
        }
        if (lexer->input[lexer->pos] == 34)
        {
            lexer->pos++;
            while (lexer->input[lexer->pos] == '\0'
                   && lexer->input[lexer->pos] != 34)
            {
                if (lexer->input[lexer->pos] == 92
                    && (lexer->input[lexer->pos + 1] == 39
                        || lexer->input[lexer->pos + 1] == 34
                        || lexer->input[lexer->pos + 1] == '\0'
                        || lexer->input[lexer->pos + 1] == '\n'
                        || lexer->input[lexer->pos + 1] == '\\'
                        || lexer->input[lexer->pos + 1] == '$'
                        || lexer->input[lexer->pos + 1] == '`'))
                {
                    lexer->pos++;
                    if (lexer->input[lexer->pos] == '\0')
                    {
                        *status = PARSER_UNEXPECTED_TOKEN;
                        break;
                    }
                    res[i] = lexer->input[lexer->pos];
                    i++;
                    lexer->pos++;
                }
                else
                {
                    res[i] = lexer->input[lexer->pos];
                    i++;
                    lexer->pos++;
                }
            }
            lexer->pos++;
        }
        else
        {
            break;
        }
    }

    // lexer->pos += 1;

    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = FUNCTION;
    tok->value = res;
    tok->string = 1;
    if (var == 1)
    {
        tok->type = VARIABLE;
    }

    return tok;
}*/

static struct token *__lexer_equal(struct lexer *lexer, char *str)
{
    free(str);
    lexer->pos += 1;

    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = EQUAL;
    tok->value = calloc(1, 100);
    tok->value[0] = '=';
    tok->value[1] = '\0';

    if (lexer->input[lexer->pos] == ' ' || lexer->input[lexer->pos] == '\n'
        || lexer->input[lexer->pos] == ';' || lexer->input[lexer->pos] == '\t'
        || lexer->input[lexer->pos] == '\0')
    {
        tok->type = VARIABLE_SUPP;
    }

    //       struct token tok = { .type = EOL, .value = NULL };
    return tok;
}

/*
struct dollar_context
{
    struct token *tok;
    size_t i;
    size_t error;
};

static void __lexer_dollar_part1(enum parser_status *status,
                                 struct lexer *lexer, char *str,
                                 struct dollar_context *ctx)
{
    free(str);
    lexer->pos++;

    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = VARIABLE;
    tok->value = calloc(1, 10000);
    size_t i = 0;
    size_t error = 0;

    tok->string = 1;

    if ((lexer->pos - 1 > 0)
        && (lexer->input[lexer->pos - 1] == '\''
            || lexer->input[lexer->pos - 1] == '\"')
        && (lexer->input[lexer->pos - 1] != '\''
            && lexer->input[lexer->pos - 1] != '\"'))
    {
        tok->type = VARIABLE_LEFT;
    }

    if (lexer->input[lexer->pos] == '{')
    {
        lexer->pos++;
        error = 1;
    }

    ctx->tok = tok;
    ctx->i = i;
    ctx->error = error;
    (void)status;
}

static void __lexer_dollar_part2(enum parser_status *status,
                                 struct lexer *lexer,
                                 struct dollar_context *ctx)
{
    struct token *tok = ctx->tok;
    size_t i = ctx->i;
    size_t error = ctx->error;

    while (
        lexer->input[lexer->pos] != '\t' && lexer->input[lexer->pos] != '\n'
        && lexer->input[lexer->pos] != ';' && lexer->input[lexer->pos] != '\''
        && lexer->input[lexer->pos] != '\"' && lexer->input[lexer->pos] != ' '
        && lexer->input[lexer->pos] != '$' && lexer->input[lexer->pos] != '`'
        && lexer->input[lexer->pos] != '|' && lexer->input[lexer->pos] != '&'
        && lexer->input[lexer->pos] != '\0' && lexer->input[lexer->pos] != '}')
    {
        tok->value[i] = lexer->input[lexer->pos];
        lexer->pos++;
        i++;
    }

    ctx->tok = tok;
    ctx->i = i;
    ctx->error = error;
    (void)status;
}

static struct token *__lexer_dollar_part3(enum parser_status *status,
                                          struct lexer *lexer,
                                          struct dollar_context *ctx)
{
    struct token *tok = ctx->tok;
    size_t i = ctx->i;
    size_t error = ctx->error;

    if (lexer->input[lexer->pos] != '}' && error == 1)
    {
        *status = PARSER_UNEXPECTED_TOKEN;
    }

    if ((lexer->input[lexer->pos] == '$' || lexer->input[lexer->pos] == '\''
         || lexer->input[lexer->pos] == '\"'))
    {
        tok->type = VARIABLE_RIGHT;
    }

    if (error == 1)
    {
        lexer->pos++;
    }

    tok->value[i] = '\0';
    return tok;
}

static struct token *__lexer_dollar(enum parser_status *status,
                                    struct lexer *lexer, char *str)
{
    struct dollar_context ctx;
    ctx.tok = NULL;
    ctx.i = 0;
    ctx.error = 0;

    __lexer_dollar_part1(status, lexer, str, &ctx);

    __lexer_dollar_part2(status, lexer, &ctx);

    return __lexer_dollar_part3(status, lexer, &ctx);
}*/

static struct token *__lexer_dollar(enum parser_status *status,
                                    struct lexer *lexer, char *str)
{
    free(str);
    lexer->pos++;

    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = VARIABLE;
    tok->value = calloc(1, 10000);
    size_t i = 0;
    size_t error = 0;

    tok->string = 1;

    if ((lexer->pos - 1 > 0)
        && (lexer->input[lexer->pos - 1] == '\''
            || lexer->input[lexer->pos - 1] == '\"')
        && (lexer->input[lexer->pos - 1] != '\''
            && lexer->input[lexer->pos - 1] != '\"'))
    {
        tok->type = VARIABLE_LEFT;
    }

    if (lexer->input[lexer->pos] == '{')
    {
        lexer->pos++;
        error = 1;
    }

    while (
        /* lexer->input[lexer->pos] != '\t' && lexer->input[lexer->pos] != '\n'
         && lexer->input[lexer->pos] != ';' && lexer->input[lexer->pos] != '\''
         && lexer->input[lexer->pos] != '\"' && lexer->input[lexer->pos] != ' '
         && lexer->input[lexer->pos] != '$' && lexer->input[lexer->pos] != '`'
         && lexer->input[lexer->pos] != '|' && lexer->input[lexer->pos] != '&'
         && lexer->input[lexer->pos] != '\0' && lexer->input[lexer->pos] !=
         '}'*/
        (lexer->input[lexer->pos] >= 'A' && lexer->input[lexer->pos] <= 'Z')
        || (lexer->input[lexer->pos] >= 'a' && lexer->input[lexer->pos] <= 'z')
        || (lexer->input[lexer->pos] >= '0' && lexer->input[lexer->pos] <= '9')
        || (lexer->input[lexer->pos] == '+' || lexer->input[lexer->pos] == '_'))
    {
        tok->value[i] = lexer->input[lexer->pos];
        lexer->pos++;
        i++;
    }

    if (lexer->input[lexer->pos] != '}' && error == 1)
    {
        *status = PARSER_UNEXPECTED_TOKEN;
    }

    /*if (lexer->input[lexer->pos] == '}')
     { lexer->pos++;}*/

    if ((lexer->input[lexer->pos] == '$' || lexer->input[lexer->pos] == '\''
         || lexer->input[lexer->pos] == '\"'))
    {
        tok->type = VARIABLE_RIGHT;
    }

    if (error == 1)
    {
        lexer->pos++;
    }

    // lexer->pos += strlen(tok->value);
    tok->value[i] = '\0';
    //       struct token tok = { .type = EOL, .value = NULL };
    return tok;
}

static struct token *__lexer_for(struct lexer *lexer, char *str)
{
    free(str);
    lexer->pos += 3;

    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = FOR;
    tok->value = calloc(1, 100);
    tok->value[0] = '=';
    tok->value[1] = '\0';

    //       struct token tok = { .type = EOL, .value = NULL };
    return tok;
}

static struct token *__lexer_in(struct lexer *lexer, char *str)
{
    free(str);
    lexer->pos += 2;

    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = IN;
    tok->value = calloc(1, 100);
    tok->value[0] = '=';
    tok->value[1] = '\0';

    //       struct token tok = { .type = EOL, .value = NULL };
    return tok;
}

static struct token *__switchh_part1(enum parser_status *status,
                                     struct lexer *lexer, char *str)
{
    if (strcmp("if", str) == 0 && !lexer->arg)
    {
        /*free(str);
        lexer->pos += 2;
        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = IF;
        tok->value = NULL;
        //       struct token tok = { .type = IF, .value = NULL };
        return tok;*/
        return __lexer_if(lexer, str);
    }
    else if (strcmp("else", str) == 0 && !lexer->arg)
    {
        /*free(str);
        lexer->pos += 4;

        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = ELSE;
        tok->value = NULL;
        // struct token tok = { .type = ELSE, .value = NULL };
        return tok;*/
        return __lexer_else(lexer, str);
    }
    else if (strcmp("elif", str) == 0 && !lexer->arg)
    {
        /*free(str);
        lexer->pos += 4;

        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = ELIF;
        tok->value = NULL;

        //        struct token tok = { .type = ELIF, .value = NULL };
        return tok;*/
        return __lexer_elif(lexer, str);
    }
    else if (strcmp("then", str) == 0 && !lexer->arg)
    {
        /*free(str);
        lexer->pos += 4;

        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = THEN;
        tok->value = NULL;

        //       struct token tok = { .type = THEN, .value = NULL };
        return tok;*/
        return __lexer_then(lexer, str);
    }
    else if (strcmp("fi", str) == 0 && !lexer->arg)
    {
        /*free(str);
        lexer->pos += 2;

        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = FI;
        tok->value = NULL;

        //       struct token tok = { .type = FI, .value = NULL };
        return tok;*/
        return __lexer_fi(lexer, str);
    }
    else if (strcmp("'", str) == 0)
    {
        /*lexer->pos += 1;

        free(str);
        char *res = calloc(1, 10000);
        clear(res);
        size_t i = 0;
        while (lexer->input[lexer->pos] != 39) //   '
        {
            res[i] = lexer->input[lexer->pos];
            i++;
            lexer->pos++;
        }

        lexer->pos += 1;

        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = FUNCTION;
        tok->value = res;

        //       struct token tok = { .type = SINGLE_QUOTE, .value = NULL };
        return tok;*/
        return __lexer_quote(status, lexer, str);
    }
    else if (strcmp("\"", str) == 0)
    {
        return __lexer_double_quote(status, lexer, str);
    }
    else if (strcmp(";", str) == 0 && lexer->virg_scape == 0)
    {
        /*free(str);
        lexer->pos += 1;

        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = EOL;
        tok->value = NULL;

        //       struct token tok = { .type = EOL, .value = NULL };
        return tok;*/
        lexer->arg_for = 0;
        lexer->arg = 0;
        return __lexer_virgule(lexer, str);
    }
    else if (strcmp("|", str) == 0 && !lexer->escap)
    {
        /*free(str);
        lexer->pos += 1;

        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = EOL;
        tok->value = NULL;

        //       struct token tok = { .type = EOL, .value = NULL };
        return tok;*/
        return __lexer_pipe(lexer, str);
    }
    else if (strcmp("!", str) == 0 && !lexer->arg)
    {
        /*free(str);
        lexer->pos += 1;

        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = EOL;
        tok->value = NULL;

        //       struct token tok = { .type = EOL, .value = NULL };
        return tok;*/
        return __lexer_negation(lexer, str);
    }
    else if (strcmp("while", str) == 0 && !lexer->arg)
    {
        /*free(str);
        lexer->pos += 1;

        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = EOL;
        tok->value = NULL;

        //       struct token tok = { .type = EOL, .value = NULL };
        return tok;*/
        return __lexer_while(lexer, str);
    }
    else if (strcmp("until", str) == 0 && !lexer->arg)
    {
        /*free(str);
        lexer->pos += 1;

        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = EOL;
        tok->value = NULL;

        //       struct token tok = { .type = EOL, .value = NULL };
        return tok;*/
        return __lexer_until(lexer, str);
    }
    else if (strcmp("do", str) == 0 && !lexer->arg)
    {
        /*free(str);
        lexer->pos += 1;

        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = EOL;
        tok->value = NULL;

        //       struct token tok = { .type = EOL, .value = NULL };
        return tok;*/
        return __lexer_do(lexer, str);
    }
    else if (strcmp("done", str) == 0 && !lexer->arg)
    {
        /*free(str);
        lexer->pos += 1;

        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = EOL;
        tok->value = NULL;

        //       struct token tok = { .type = EOL, .value = NULL };
        return tok;*/
        return __lexer_done(lexer, str);
    }

    (void)status;
    return NULL;
}

static struct token *__switchh_part2(enum parser_status *status,
                                     struct lexer *lexer, char *str)
{
    if (strcmp("||", str) == 0)
    {
        /*free(str);
        lexer->pos += 1;

        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = EOL;
        tok->value = NULL;

        //       struct token tok = { .type = EOL, .value = NULL };
        return tok;*/
        return __lexer_or(lexer, str);
    }
    else if (strcmp("&&", str) == 0)
    {
        /*free(str);
        lexer->pos += 1;

        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = EOL;
        tok->value = NULL;

        //       struct token tok = { .type = EOL, .value = NULL };
        return tok;*/
        return __lexer_and(lexer, str);
    }
    else if (strcmp("=", str) == 0)
    {
        /*free(str);
        lexer->pos += 1;

        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = EOL;
        tok->value = NULL;

        //       struct token tok = { .type = EOL, .value = NULL };
        return tok;*/
        return __lexer_equal(lexer, str);
    }
    else if (strcmp("`", str) == 0)
    {
        /*free(str);
        lexer->pos += 1;

        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = EOL;
        tok->value = NULL;

        //       struct token tok = { .type = EOL, .value = NULL };
        return tok;*/
        return __lexer_quote_chelou(lexer, str);
    }
    else if (strcmp("$", str) == 0)
    {
        /*free(str);
        lexer->pos += 1;

        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = EOL;
        tok->value = NULL;

        //       struct token tok = { .type = EOL, .value = NULL };
        return tok;*/
        return __lexer_dollar(status, lexer, str);
    }
    else if (strcmp("for", str) == 0 /*&& !lexer->arg*/)
    {
        /*free(str);
        lexer->pos += 1;

        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = EOL;
        tok->value = NULL;

        //       struct token tok = { .type = EOL, .value = NULL };
        return tok;*/
        lexer->arg_for = 1;
        return __lexer_for(lexer, str);
    }
    else if (strcmp("in", str) == 0 && lexer->arg_for == 1)
    {
        /*free(str);
        lexer->pos += 1;

        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = EOL;
        tok->value = NULL;

        //       struct token tok = { .type = EOL, .value = NULL };
        return tok;*/
        //   lexer->arg_for = 0;
        return __lexer_in(lexer, str);
    }

    return NULL;
}

struct switchh_part3_ctx
{
    struct lexer *lexer;
    char *str;
    char *t;
};

static struct token *__switchh_part3_A(struct switchh_part3_ctx *ctx)
{
    ctx->lexer->arg = 1;
    ctx->lexer->escap = 0;
    // char *t = determined_type(lexer);
    char *t = ctx->str;
    ctx->lexer->virg_scape = 0;

    if (t[0] == '>' || t[0] == '<')
    {
        struct token *tok = malloc(sizeof(struct token));
        tok->type = REDIRECTION;
        tok->value = t;
        ctx->lexer->pos += strlen(t);

        return tok;
    }
    if ((ctx->lexer->input[ctx->lexer->pos + 1] == '>'
         || ctx->lexer->input[ctx->lexer->pos + 1] == '<')
        && ctx->lexer->input[ctx->lexer->pos + 1] != '\0')
    {
        struct token *tok = malloc(sizeof(struct token));
        tok->type = IONUMBER;
        tok->value = t;
        ctx->lexer->pos += strlen(t);

        return tok;
    }

    ctx->t = t;
    return (struct token *)-1;
}

static struct token *__switchh_part3_B(struct switchh_part3_ctx *ctx)
{
    struct lexer *lexer = ctx->lexer;
    char *t = ctx->t;

    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = FUNCTION;
    tok->value = t;

    size_t mid = 0;
    size_t mid2 = 0;
    if (lexer->pos > 2 && lexer->input[lexer->pos - 1] == '\'')
    {
        mid++;
    }
    // printf("pos %lu\n",lexer->pos);
    if (lexer->pos > 2
        && (lexer->input[lexer->pos - 1] == '\"'
            || lexer->input[lexer->pos - 2] == '\"')
        && lexer->input[lexer->pos - 1] != ' '
        && lexer->input[lexer->pos - 1] != '\t')
    { // printf("gfergergerg\n");
        mid2++;
        tok->type = FUNCTION_LEFT;
    }

    if ((lexer->pos >= 2)
        && (lexer->input[lexer->pos - 1] == '\''
            || lexer->input[lexer->pos - 1] == '\"')
        /*&& (lexer->input[lexer->pos - 2] != '\''
            && lexer->input[lexer->pos - 2] != '\"'
            && lexer->input[lexer->pos - 2] != '$')*/)
    {
        tok->type = FUNCTION_LEFT;
    }

    lexer->pos += strlen(t);

    if ((lexer->input[lexer->pos] == '\'' || lexer->input[lexer->pos] == '\"')
        && (lexer->input[lexer->pos + 1] != '\''
            && lexer->input[lexer->pos + 1] != '\"'
            && lexer->input[lexer->pos + 1] != '$'))
    {
        tok->type = FUNCTION_RIGHT;
    }

    if (lexer->input[lexer->pos] != '\0' && lexer->input[lexer->pos + 1] != '\0'
        && (lexer->input[lexer->pos + 1] == '\''
            || lexer->input[lexer->pos] == '\'')
        && mid == 1)
    {
        tok->type = FUNCTION_MIDDLE;
    }

    if (lexer->input[lexer->pos] != '\0' && lexer->input[lexer->pos + 1] != '\0'
        && (lexer->input[lexer->pos + 2] == '\"'
            || lexer->input[lexer->pos + 1] == '\"')
        && mid2 == 1)
    {
        tok->type = FUNCTION_MIDDLE;
    }

    return tok;
}

static struct token *__switchh_part3(struct lexer *lexer, char *str)
{
    struct switchh_part3_ctx ctx;
    ctx.lexer = lexer;
    ctx.str = str;
    ctx.t = NULL;

    struct token *ret = __switchh_part3_A(&ctx);
    if (ret != (struct token *)-1)
    {
        return ret;
    }

    ret = __switchh_part3_B(&ctx);
    return ret;
}

/*
static struct token *__switchh_part3(struct lexer *lexer, char *str)
{
    // free(str);
    lexer->arg = 1;
    lexer->escap = 0;
    // char *t = determined_type(lexer);
    char *t = str;
    lexer->virg_scape = 0;

    if ((lexer->input[lexer->pos + 1] == '>'
         || lexer->input[lexer->pos + 1] == '<')
        && lexer->input[lexer->pos + 1] != '\0')
    {
        struct token *tok = malloc(sizeof(struct token));
        tok->type = IONUMBER;
        tok->value = t;
        lexer->pos += strlen(t);

        return tok;
    }
    if (t[0] == '>' || t[1] == '<')
    {
        struct token *tok = malloc(sizeof(struct token));
        tok->type = REDIRECTION;
        tok->value = t;
        lexer->pos += strlen(t);

        return tok;
    }

    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = FUNCTION;
    tok->value = t;

    size_t mid = 0;
    size_t mid2 = 0;
    if (lexer->pos > 2 && lexer->input[lexer->pos - 1] == '\'')
    {
        mid++;
    }
    // printf("pos %lu\n",lexer->pos);
    if (lexer->pos > 2
        && (lexer->input[lexer->pos - 1] == '\"'
            || lexer->input[lexer->pos - 2] == '\"')
        && lexer->input[lexer->pos - 1] != ' '
        && lexer->input[lexer->pos - 1] != '\t')
    { // printf("gfergergerg\n");
        mid2++;
        tok->type = FUNCTION_LEFT;
    }

    if ((lexer->pos >= 2)
        && (lexer->input[lexer->pos - 1] == '\''
            || lexer->input[lexer->pos - 1] == '\"')
        && (lexer->input[lexer->pos - 2] != '\''
            && lexer->input[lexer->pos - 2] != '\"'
            && lexer->input[lexer->pos - 2] != '$'))
    {
        tok->type = FUNCTION_LEFT;
    }

    lexer->pos += strlen(t);

    if ((lexer->input[lexer->pos] == '\'' || lexer->input[lexer->pos] == '\"')
        && (lexer->input[lexer->pos + 1] != '\''
            && lexer->input[lexer->pos + 1] != '\"'
            && lexer->input[lexer->pos + 1] != '$'))
    {
        tok->type = FUNCTION_RIGHT;
    }

    if (lexer->input[lexer->pos] != '\0' && lexer->input[lexer->pos + 1] != '\0'
        && (lexer->input[lexer->pos + 1] == '\''
            || lexer->input[lexer->pos] == '\'')
        && mid == 1)
    {
        tok->type = FUNCTION_MIDDLE;
    }

    if (lexer->input[lexer->pos] != '\0' && lexer->input[lexer->pos + 1] != '\0'
        && (lexer->input[lexer->pos + 2] == '\"'
            || lexer->input[lexer->pos + 1] == '\"')
        && mid2 == 1)
    {
        tok->type = FUNCTION_MIDDLE;
    }

    return tok;
}*/

static struct token *switchh(enum parser_status *status, struct lexer *lexer,
                             char *str)
{
    struct token *tok = __switchh_part1(status, lexer, str);
    if (tok)
    {
        return tok;
    }

    tok = __switchh_part2(status, lexer, str);
    if (tok)
    {
        return tok;
    }

    return __switchh_part3(lexer, str);
}

/*
static struct token *switchh(enum parser_status *status, struct lexer *lexer,
                             char *str)
{
    if (strcmp("if", str) == 0 && !lexer->arg)
    {

        return __lexer_if(lexer, str);
    }
    else if (strcmp("else", str) == 0 && !lexer->arg)
    {

        return __lexer_else(lexer, str);
    }
    else if (strcmp("elif", str) == 0 && !lexer->arg)
    {
        return __lexer_elif(lexer, str);
    }
    else if (strcmp("then", str) == 0 && !lexer->arg)
    {

        return __lexer_then(lexer, str);
    }
    else if (strcmp("fi", str) == 0 && !lexer->arg)
    {

        return __lexer_fi(lexer, str);
    }
    else if (strcmp("'", str) == 0)
    {

        return __lexer_quote(status, lexer, str);
    }
    else if (strcmp("\"", str) == 0)
    {
        return __lexer_double_quote(status, lexer, str);
    }
    else if (strcmp(";", str) == 0)
    {

        lexer->arg = 0;
        return __lexer_virgule(lexer, str);
    }
    else if (strcmp("|", str) == 0 && !lexer->escap)
    {
        return __lexer_pipe(lexer, str);
    }
    else if (strcmp("!", str) == 0 && !lexer->arg)
    {

        return __lexer_negation(lexer, str);
    }
    else if (strcmp("while", str) == 0 && !lexer->arg)
    {

        return __lexer_while(lexer, str);
    }
    else if (strcmp("until", str) == 0 && !lexer->arg)
    {

        return __lexer_until(lexer, str);
    }
    else if (strcmp("do", str) == 0 && !lexer->arg)
    {

        return __lexer_do(lexer, str);
    }
    else if (strcmp("done", str) == 0 && !lexer->arg)
    {

        return __lexer_done(lexer, str);
    }
    else if (strcmp("||", str) == 0)
    {

        return __lexer_and(lexer, str);
    }
    else if (strcmp("&&", str) == 0)
    {

        return __lexer_or(lexer, str);
    }
    else if (strcmp("=", str) == 0)
    {

        return __lexer_equal(lexer, str);
    }
    else if (strcmp("`", str) == 0)
    {

        return __lexer_quote_chelou(lexer, str);
    }
    else if (strcmp("$", str) == 0)
    {

        return __lexer_dollar(status, lexer, str);
    }
    else if (strcmp("for", str) == 0)
    {

        return __lexer_for(lexer, str);
    }
    else if (strcmp("in", str) == 0)
    {

        return __lexer_in(lexer, str);
    }
    else
    {
        // free(str);
        lexer->arg = 1;
        lexer->escap = 0;
        // char *t = determined_type(lexer);
        char *t = str;

        struct token *tok = calloc(1, sizeof(struct token));
        tok->type = FUNCTION;
        tok->value = t;

        if ((lexer->pos >= 2)
            && (lexer->input[lexer->pos - 1] == '\''
                || lexer->input[lexer->pos - 1] == '\"')
            && (lexer->input[lexer->pos - 2] != '\''
                && lexer->input[lexer->pos - 2] != '\"'
                && lexer->input[lexer->pos - 2] != '$'))
        {
            tok->type = FUNCTION_LEFT;
        }

        lexer->pos += strlen(t);

        if ((lexer->input[lexer->pos] == '\''
             || lexer->input[lexer->pos] == '\"')
            && (lexer->input[lexer->pos + 1] != '\''
                && lexer->input[lexer->pos + 1] != '\"'
                && lexer->input[lexer->pos + 1] != '$'))
        {
            tok->type = FUNCTION_RIGHT;
        }

        //        struct token tok = { .type = FUNCTION, .value = t};
        // printf("STRLEN : %lu  \n", strlen(t));
        // free(t);
        return tok;
    }
}*/

struct token *lexer_next_token(enum parser_status *status, struct lexer *lexer)
{
    if (lexer->pos == 0)
    {
        lexer->arg2 = 0;
    }
    if (lexer->input[lexer->pos] == '\0')
    {
        /*struct token *tok = calloc(1, sizeof(struct token));
        tok->type = EOFF;
        tok->value = NULL;
        // struct token tok = { .type = EOFF, .value = NULL };
        return tok;*/
        return __lexer_fin();
    }

    if (lexer->input[lexer->pos] == ' ' || lexer->input[lexer->pos] == '\t')
    {
        ++lexer->pos;
        return lexer_next_token(status, lexer);
    }

    if (lexer->input[lexer->pos] == '#')
    {
        while (lexer->input[lexer->pos] != '\n'
               && lexer->input[lexer->pos] != '\0')
        {
            ++lexer->pos;
        }

        if (lexer->arg2 == 0)
        {
            char c = lexer->input[lexer->pos];
            while ((c == '\n' || c == ' ' || c == '\t') && c != '\0')
            {
                lexer->pos++;
                c = lexer->input[lexer->pos];
            }
        }
        if (lexer->input[lexer->pos] == '\0')
        {
            lexer->pos--;
            struct token *tok = calloc(1, sizeof(struct token));
            tok->type = EOFF;
            tok->value = NULL;
            return tok;
        }

        // lexer->input[lexer->pos] = '\n';

        //       ++lexer->pos;
    }

    if (lexer->input[lexer->pos] == '\n')
    {
        /*struct token *tok = calloc(1, sizeof(struct token));
        tok->type = EOL;
        tok->value = NULL;
        lexer->pos += 1;
        //        struct token tok = { .type = EOL, .value = NULL };
        return tok;*/
        lexer->arg = 0;
        return __lexer_nnn(lexer);
    }

    char *str = determined_type(lexer);

    return switchh(status, lexer, str);
}

/**
 * \brief Returns the next token, but doesn't move forward: calling lexer_peek
 * multiple times in a row always returns the same result.
 * This function is meant to help the parser check if the next token matches
 * some rule.
 */
struct token *lexer_peek(enum parser_status *status, struct lexer *lexer)
{
    size_t tmp = lexer->pos;
    if (lexer->pos == 0)
    {
        lexer->arg = 0;
    }
    struct token *res = lexer_next_token(status, lexer);

    lexer->pos = tmp;

    return res;
}

/**
 * \brief Returns the next token, and removes it from the stream:
 *   calling lexer_pop in a loop will iterate over all tokens until EOF.
 */
void lexer_pop(enum parser_status *status, struct lexer *lexer)
{
    struct token *tok = lexer_next_token(status, lexer);

    if (lexer->current_tok->value == NULL && tok->value != NULL)
    {
        lexer->current_tok->value = calloc(1, 100000);
        strcpy(lexer->current_tok->value, tok->value);
    }

    lexer->current_tok->type = tok->type;
    token_free(tok);

    lexer->arg2 = 1;
    //  return tok;
}

void token_free(struct token *tok)
{
    if (tok->value != NULL)
    {
        free(tok->value);
    }
    free(tok);
}
