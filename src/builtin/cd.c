#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "eval.h"

int cd(char *path)
{
    if (!path || strcmp(path, "") == 0)
    {
        char *home = getenv("HOME");
        if (!home)
        {
            return 0;
        }
        char *pwd = getenv("PWD");
        int ret = chdir(home);
        if (ret == -1)
        {
            fprintf(stderr, "erreur changement directory");
            return 1;
        }
        char buff[4096];
        setenv("PWD", getcwd(buff, 4096), 1);
        if (pwd)
        {
            setenv("OLDPWD", pwd, 1);
        }
        return 0;
    }
    else if (strcmp(path, "-") == 0)
    {
        char *pwd = getenv("PWD");
        char *oldpwd = getenv("OLDPWD");
        printf("%s\n", oldpwd);
        int ret = chdir(oldpwd);
        if (ret == -1)
        {
            fprintf(stderr, "erreur changement directory");
            return 1;
        }
        char buff[4096];
        setenv("PWD", getcwd(buff, 4096), 1);

        if (pwd)
        {
            setenv("OLDPWD", pwd, 1);
        }
        return 0;
    }
    char *pwd = getenv("PWD");
    int ret = chdir(path);
    if (ret == -1)
    {
        fprintf(stderr, "erreur changement directory");
        return 1;
    }
    char buff[4096];
    setenv("PWD", getcwd(buff, 4096), 1);
    if (pwd)
    {
        setenv("OLDPWD", pwd, 1);
    }
    return 0;
}
