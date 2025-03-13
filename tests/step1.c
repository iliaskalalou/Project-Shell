#include <criterion/criterion.h>
#include <criterion/internal/assert.h>
#include <criterion/internal/new_asserts.h>
#include <criterion/new/assert.h>
#include <stddef.h>

#include "lexer.h"
#include "token.h"

struct test_case
{
    char *val;
    enum token_type *expected_token;
    size_t nb_token;
};

#define NB_TEST(tests) sizeof(tests) / sizeof(tests[0])

static char *get_token_name(enum token_type type)
{
    switch (type)
    {
    case FUNCTION:
        return "FUNCTION";
    case EOL:
        return "EOL";
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
    case EOFF:
        return "EOFF";
    case AST_LIST:
        return "AST_LIST";
    case ERREUR:
        return "ERREUR";
    }
}

TestSuite(Step1);

Test(Step1, lexer_empty)
{
    struct lexer *lex = lexer_new("");
    cr_expect_not_null(lex);
    struct token *a = lexer_next_token(lex);
    cr_expect(a->type == EOFF, "Expected EOFF got %s", get_token_name(a->type));
    free(a);
    lexer_free(lex);
}

Test(Step1, lexer_null)
{
    struct lexer *lex = lexer_new(NULL);
    cr_expect_null(lex);
}

Test(Step1, lexer_simple_func)
{
    char *strings[] = { "ls",   "echo", "if",   ";", "then",
                        "else", "fi",   "elif", "'" };
    for (size_t i = 0; i < sizeof(strings) / sizeof(strings[0]); i++)
    {
        struct lexer *lex = lexer_new(strings[i]);
        cr_expect_not_null(lex, "failed on test %zu: %s", i, strings[i]);
        struct token *tok = lexer_next_token(lex);
        if (i < 2)
        {
            cr_expect(tok->type == FUNCTION,
                      "failed on test %zu: %s expected %i got %i", i,
                      strings[i], FUNCTION, tok->type);
        }
        else
        {
            cr_expect(tok->type != FUNCTION, "failed on test %zu: %s got %i", i,
                      strings[i], FUNCTION);
        }
        free(tok);
        lexer_free(lex);
    }
}

Test(Step1, lexer_simple_if)
{
    char *strings[] = { "ls",   "echo", "if",   ";", "then",
                        "else", "fi",   "elif", "'" };
    for (size_t i = 0; i < sizeof(strings) / sizeof(strings[0]); i++)
    {
        struct lexer *lex = lexer_new(strings[i]);
        cr_expect_not_null(lex, "failed on test %zu: %s", i, strings[i]);
        struct token *tok = lexer_next_token(lex);
        if (i == 2)
        {
            cr_expect(tok->type == IF,
                      "failed on test %zu: %s expected %i got %i", i,
                      strings[i], IF, tok->type);
        }
        else
        {
            cr_expect(tok->type != IF, "failed on test %zu: %s got %i", i,
                      strings[i], tok->type);
        }
        free(tok);
        lexer_free(lex);
    }
}

Test(Step1, lexer_simple_eol)
{
    char *strings[] = { "ls",   "echo", "if",   ";", "then",
                        "else", "fi",   "elif", "'" };
    for (size_t i = 0; i < sizeof(strings) / sizeof(strings[0]); i++)
    {
        struct lexer *lex = lexer_new(strings[i]);
        cr_expect_not_null(lex, "failed on test %zu: %s", i, strings[i]);
        struct token *tok = lexer_next_token(lex);
        if (i == 3)
        {
            cr_expect(tok->type == EOL,
                      "failed on test %zu: %s expected %i got %i", i,
                      strings[i], EOL, tok->type);
        }
        else
        {
            cr_expect(tok->type != EOL, "failed on test %zu: %s got %i", i,
                      strings[i], tok->type);
        }
        lexer_free(lex);
    }
}

Test(Step1, lexer_simple_then)
{
    char *strings[] = { "ls",   "echo", "if",   ";", "then",
                        "else", "fi",   "elif", "'" };
    for (size_t i = 0; i < sizeof(strings) / sizeof(strings[0]); i++)
    {
        struct lexer *lex = lexer_new(strings[i]);
        cr_expect_not_null(lex, "failed on test %zu: %s", i, strings[i]);
        struct token *tok = lexer_next_token(lex);
        if (i == 4)
        {
            cr_expect(tok->type == THEN,
                      "failed on test %zu: %s expected %i got %i", i,
                      strings[i], THEN, tok->type);
        }
        else
        {
            cr_expect(tok->type != THEN, "failed on test %zu: %s got %i", i,
                      strings[i], tok->type);
        }
        lexer_free(lex);
    }
}

Test(Step1, lexer_simple_else)
{
    char *strings[] = { "ls",   "echo", "if",   ";", "then",
                        "else", "fi",   "elif", "'" };
    for (size_t i = 0; i < sizeof(strings) / sizeof(strings[0]); i++)
    {
        struct lexer *lex = lexer_new(strings[i]);
        cr_expect_not_null(lex, "failed on test %zu: %s", i, strings[i]);
        struct token *tok = lexer_next_token(lex);
        if (i == 5)
        {
            cr_expect(tok->type == ELSE,
                      "failed on test %zu: %s expected %i got %i", i,
                      strings[i], ELSE, tok->type);
        }
        else
        {
            cr_expect(tok->type != ELSE, "failed on test %zu: %s got %i", i,
                      strings[i], tok->type);
        }
        lexer_free(lex);
    }
}

Test(Step1, lexer_simple_fi)
{
    char *strings[] = { "ls",   "echo", "if",   ";", "then",
                        "else", "fi",   "elif", "'" };
    for (size_t i = 0; i < sizeof(strings) / sizeof(strings[0]); i++)
    {
        struct lexer *lex = lexer_new(strings[i]);
        cr_expect_not_null(lex, "failed on test %zu: %s", i, strings[i]);
        struct token *tok = lexer_next_token(lex);
        if (i == 6)
        {
            cr_expect(tok->type == FI,
                      "failed on test %zu: %s expected %i got %i", i,
                      strings[i], FI, tok->type);
        }
        else
        {
            cr_expect(tok->type != FI, "failed on test %zu: %s got %i", i,
                      strings[i], tok->type);
        }
        lexer_free(lex);
    }
}

Test(Step1, lexer_simple_elif)
{
    char *strings[] = { "ls",   "echo", "if",   ";", "then",
                        "else", "fi",   "elif", "'" };
    for (size_t i = 0; i < sizeof(strings) / sizeof(strings[0]); i++)
    {
        struct lexer *lex = lexer_new(strings[i]);
        cr_expect_not_null(lex, "failed on test %zu: %s", i, strings[i]);
        struct token *tok = lexer_next_token(lex);
        if (i == 7)
        {
            cr_expect(tok->type == ELIF,
                      "failed on test %zu: %s expected %i got %i", i,
                      strings[i], ELIF, tok->type);
        }
        else
        {
            cr_expect(tok->type != ELIF, "failed on test %zu: %s got %i", i,
                      strings[i], tok->type);
        }
        lexer_free(lex);
    }
}

Test(Step1, lexer_simple_single_quote)
{
    char *strings[] = { "ls",   "echo", "if",   ";", "then",
                        "else", "fi",   "elif", "'" };
    for (size_t i = 0; i < sizeof(strings) / sizeof(strings[0]); i++)
    {
        struct lexer *lex = lexer_new(strings[i]);
        cr_expect_not_null(lex, "failed on test %zu: %s", i, strings[i]);
        struct token *tok = lexer_next_token(lex);
        if (i == 8)
        {
            cr_expect(tok->type == SINGLE_QUOTE,
                      "failed on test %zu: %s expected %i got %i", i,
                      strings[i], SINGLE_QUOTE, tok->type);
        }
        else
        {
            cr_expect(tok->type != SINGLE_QUOTE,
                      "failed on test %zu: %s got %i", i, strings[i],
                      tok->type);
        }
        lexer_free(lex);
    }
}

Test(Step1, lexer_mix)
{
    enum token_type rep1[] = { FUNCTION, EOL, IF, THEN, ELSE, EOFF };
    enum token_type rep2[] = {
        FUNCTION,     FUNCTION,     FUNCTION, SINGLE_QUOTE, FUNCTION,
        SINGLE_QUOTE, EOL,          IF,       FUNCTION,     THEN,
        FUNCTION,     SINGLE_QUOTE, FUNCTION, SINGLE_QUOTE, EOL,
        FUNCTION,     SINGLE_QUOTE, FUNCTION, SINGLE_QUOTE, ELIF,
        FUNCTION,     SINGLE_QUOTE, FUNCTION, SINGLE_QUOTE, ELSE,
        FUNCTION,     SINGLE_QUOTE, FUNCTION, SINGLE_QUOTE, FI,
        EOFF
    };

    struct test_case tests[] = {
        { .val = "ls; if then else",
          .expected_token = rep1,
          .nb_token = sizeof(rep1) / sizeof(rep1[0]) },

        { .val = "echo -n -e 'test'; if false then echo 'false'; echo 'ok' "
                 "elif echo 'maybe' else echo 'ko' fi",
          .expected_token = rep2,
          .nb_token = sizeof(rep2) / sizeof(rep2[0]) }
    };

    for (size_t i = 0; i < NB_TEST(tests); i++)
    {
        struct lexer *lex = lexer_new(tests[i].val);
        cr_expect_not_null(lex, "failed on test %zu: %s", i, tests[i].val);
        for (size_t j = 0; j < tests[i].nb_token; j++)
        {
            struct token *tok = lexer_next_token(lex);
            cr_expect(tok->type == tests[i].expected_token[j],
                      "failed on test %zu: %s at %zu expected token %s got %s",
                      i, tests[i].val, j,
                      get_token_name(tests[i].expected_token[j]),
                      get_token_name(tok->type));
        }
        lexer_free(lex);
    }
}
