#ifndef LEXER_H
#define LEXER_H

#include "token.h"

/**
 * \page Lexer
 *
 * The lexer cuts some input text into blocks called tokens.

 * This process is done **on demand**: the lexer doesn't read the
 * input more than it needs, only creates tokens when lexer_peek
 * or lexer_pop is called, and no token is available.
 *
 * "2 + 3" will produce 3 tokens:
 *   - TOKEN_NUMBER { .value = 2 }
 *   - TOKEN_PLUS
 *   - TOKEN_NUMBER { .value = 3 }
 */

struct lexer
{
    char *input; // The input data
    size_t pos; // The current offset inside the input data
    struct token *current_tok; // The next token, if processed
    size_t arg;
    size_t arg2;
    size_t escap;
    size_t arg_for;
    size_t taille;
    size_t virg_scape;
};

enum parser_status
{
    PARSER_OK,
    PARSER_UNEXPECTED_TOKEN,
};

/**
 * \brief Creates a new lexer given an input string.
 */
struct lexer *lexer_new(char *input);

/**
 ** \brief Frees the given lexer, but not its input.
 */
void lexer_free(struct lexer *lexer);

/**
 * \brief Returns a token from the input string.

 * This function goes through the input string character by character and
 * builds a token. lexer_peek and lexer_pop should call it. If the input is
 * invalid, you must print something on stderr and return the appropriate token.
 */
struct token *lexer_next_token(enum parser_status *status, struct lexer *lexer);

/**
 * \brief Returns the next token, but doesn't move forward: calling lexer_peek
 * multiple times in a row always returns the same result.
 * This function is meant to help the parser check if the next token matches
 * some rule.
 */
struct token *lexer_peek(enum parser_status *status, struct lexer *lexer);

/**
 * \brief Returns the next token, and removes it from the stream:
 *   calling lexer_pop in a loop will iterate over all tokens until EOF.
 */
void lexer_pop(enum parser_status *status, struct lexer *lexer);

void token_free(struct token *tok);
#endif /* ! LEXER_H */
