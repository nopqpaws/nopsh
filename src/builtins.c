#include <string.h>    // strcmp
#include <stdlib.h>    // exit
#include <unistd.h>    // chdir, getcwd
#include <stdio.h>     // perror, fprintf
#include <sys/stat.h>  // chmod for getrun

#include "builtins.h"
#include "executor.h"
#include "net.h"

/*
    ----- Built‑in command implementations -----

    Built-ins run directly in the shell process instead of forking. It's
    important for things like cd, which needs to actually modify the shell's
    working directory. I previously ran into the issue where the child process 
    was changing its own cwd and not doing anything.

    Supported built‑ins:
    cd <dir>        – change the current working directory
    exit            – quit the shell
    pwd             – print working directory
    echo <args>     – print arguments to stdout
    help            – list available built-ins
    getrun <url>    – download a file via HTTP and execute it

    Anything not in this list gets passed off to the fork/exec path in executor.c

    Note: getrun calls http_download(), makes the result executable, then runs
    it through the normal executor.
*/

int is_builtin(char *cmd) {
    return strcmp(cmd, "cd") == 0     ||
           strcmp(cmd, "exit") == 0   ||
           strcmp(cmd, "pwd") == 0    ||
           strcmp(cmd, "echo") == 0   ||
           strcmp(cmd, "clear") == 0  ||
           strcmp(cmd, "help") == 0   ||
           strcmp(cmd, "getrun") == 0;
}

int run_builtin(char **argv) {
    if (strcmp(argv[0], "help") == 0) {
        printf("\033[1;32m[nopsh help]\033[0m\n");
        printf("    cd <dir>       - change directory\n");
        printf("    exit           - exit the shell\n");
        printf("    pwd            - print working directory\n");
        printf("    echo <args>    - print arguments\n");
        printf("    clear          - clear the screen\n");
        printf("    getrun <url>   - download + execute file\n");
        printf("    help           - show this menu\n");
        return 1;
    }

    if (strcmp(argv[0], "cd") == 0) {
        if (argv[1] != NULL) {
            if (chdir(argv[1]) != 0) {
                perror("cd");
            }
        } else {
            fprintf(stderr, "cd: missing argument\n");
        }
        return 1;
    } 

    if (strcmp(argv[0], "exit") == 0) {
        exit(0);
    }

    if (strcmp(argv[0], "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("pwd");
        }
        return 1;
    }

    if (strcmp(argv[0], "echo") == 0) {
        for (int i = 1; argv[i] != NULL; i++) {
            printf("%s", argv[i]);
            if (argv[i + 1] != NULL)
                printf(" ");
        }
        printf("\n");
        return 1;
    }

    if (strcmp(argv[0], "clear") == 0) {
        write(STDOUT_FILENO, "\033[H\033[J", 6);
        return 1;
    }

    if (strcmp(argv[0], "getrun") == 0) {
        if (!argv[1]) {
            fprintf(stderr, "getrun: missing URL\n");
            return 1;
        }

        const char *tmpfile = "/tmp/nopsh_download";

        if (http_download(argv[1], tmpfile) != 0) {
            fprintf(stderr, "getrun: download failed\n");
            return 1;
        }

        chmod(tmpfile, 0700);

        char *exec_argv[] = { (char *)tmpfile, NULL };
        execute(exec_argv);

        remove(tmpfile);
        return 1;
    }

    return 0; // indicate that we did not run a builtin
}