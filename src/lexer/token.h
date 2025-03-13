#ifndef TOKEN_H
#define TOKEN_H

#include <unistd.h>

enum token_type
{
    FUNCTION,
    EOL,
    EOFF,
    IF,
    THEN,
    ELIF,
    ELSE,
    FI,
    SINGLE_QUOTE,
    ERREUR,
    AST_LIST,
    PIPE,
    NEGATION,
    WHILE,
    UNTIL,
    DO,
    DONE,
    AND,
    OR,
    EXEC,
    REDIRECTION,
    IONUMBER,
    FILEM,
    FUNCTION_LEFT,
    FUNCTION_RIGHT,
    FUNCTION_MIDDLE,
    EQUAL,
    VARIABLE,
    VARIABLE_SUPP,
    VARIABLE_LEFT,
    VARIABLE_RIGHT,
    VARIABLE_EMPTY,
    FOR,
    IN
};

struct token
{
    enum token_type type; // The kind of token
    char *value; // If the token is a number, its value
    size_t string;
};

#endif /* !TOKEN_H */
