#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eval.h"

enum ECHO_OPT
{
    NEWLINE = 1,
    INTERPRETATION = 2
};

struct checker
{
    int opt;
    int option;
};

/*
 *
 * get the options in the args for the echo commande
 *   -n: disable the newline at then end
 *   -E: disable the INTERPRETATION
 *   -e: enable the INTERPRETATION
 *
 */

static int parse_opt(char **args, struct checker *pp)
{
    int i = 0;
    while (**args == '-')
    {
        (*args)++;
        i++;
    }
    while (**args != ' ')
    {
        switch (**args)
        {
        case 'n':
            pp->option = 1;
            pp->opt = pp->opt & ~NEWLINE;
            break;
        case 'e':
            pp->opt = pp->opt | INTERPRETATION;
            pp->option = 1;
            break;
        case 'E':
            pp->opt = pp->opt & ~INTERPRETATION;
            pp->option = 1;
            break;
        default:
            (*args) -= i;
            return 0;
        }
        (*args)++;
        i++;
    }
    return 1;
}

static struct checker *get_echo_opt(char **args)
{
    struct checker *pp = malloc(sizeof(struct checker));
    pp->opt = NEWLINE;
    pp->option = 0;
    int return_to = 0;
    for (; **args; (*args)++)
    {
        if (**args == ' ')
        {
            return_to++;
            continue;
        }
        if (**args != '-')
        {
            break;
        }
        if (!parse_opt(args, pp))
        {
            break;
        }
        else
        {
            return_to = 0;
        }
    }
    *args -= return_to;
    return pp;
}

static void print_interp(char *args)
{
    while (*args)
    {
        if (*args == '\\')
        {
            switch (*(args + 1))
            {
            case 'n':
                args++;
                printf("\n");
                break;
            case 't':
                args++;
                printf("\t");
                break;
            case '\\':
                printf("\\");
                args++;
                break;
            default:
                printf("\\");
                break;
            }
        }
        else
        {
            printf("%c", *args);
        }
        args++;
    }
}

int echo(char *args)
{
    char *temp = malloc(100000);
    strcpy(temp, args);

    struct checker *ooopt = get_echo_opt(&args);

    int opt = ooopt->opt;

    if (ooopt->option == 0)
    {
        if (opt & INTERPRETATION)
        {
            print_interp(temp);
        }
        else
        {
            printf("%s", temp);
        }
        if (opt & NEWLINE)
        {
            printf("\n");
        }
        free(temp);
        free(ooopt);
        return 0;
    }
    else
    {
        free(temp);
        if (opt & INTERPRETATION)
        {
            print_interp(args);
        }
        else
        {
            printf("%s", args);
        }
        if (opt & NEWLINE)
        {
            printf("\n");
        }
        free(ooopt);
        return 0;
    }
}
