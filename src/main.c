// I/O, memory management, string handling.
#include <stdio.h>     // printf, fgets
#include <stdlib.h>    // malloc, free, exit
#include <string.h>    // strtok, strcmp

// Process management and sys calls.
#include <unistd.h>    // fork, execvp, chdir
#include <sys/types.h> // pid_t
#include <sys/wait.h>  // wait, waitpid
#include <sys/stat.h>  // chmod

#include "input.h"
#include "parser.h"
#include "builtins.h"
#include "executor.h"
#include "net.h"

/*
    ----- Main entry point and REPL (Read–Eval–Print Loop) -----

    Print prompt, read input, parse it, run a built-in or fork/exec 
    an external command, repeat. Exits on Ctrl+D or the exit built-in.

    Pass --getrun <url> to skip the main loop. It will download the
    file, chmod it to 0700, execute it, delete it, then exit.
*/

int main(int argc, char **argv) { 

    // non-interactive mode: ./nopsh --getrun <url>
    if (argc == 3 && strcmp(argv[1], "--getrun") == 0) {
        const char *url = argv[2];
        const char *tmpfile = "/tmp/nopsh_download";

        if (http_download(url, tmpfile) != 0) {
            fprintf(stderr, "getrun: download failed\n");
            return 1;
        }

        chmod(tmpfile, 0700);

        char *exec_argv[] = { (char *)tmpfile, NULL };
        execute(exec_argv);

        remove(tmpfile);
        return 0;
    }

    while (1) { 

        // use colored shell prompt if possible, otherwise use basic shell prompt.
        if (isatty(STDOUT_FILENO)) {
            printf("\033[1;32m[nopsh]\033[0m ➤ ");
        } else {
            printf("[nopsh] > ");
        }

        fflush(stdout);

        char *line = read_line();
        
        // exit on EOF (Ctrl+D) or error
        if (!line) {
            printf("\n");
            break; 
        }

        char **args = tokenize(line);
        
        // check for empty input
        if (!args || !args[0]) { 
            free(line);
            free(args);
            continue;
        }

        // try to run a builtin command if it is defined in builtins.c
        if (run_builtin(args)) {
            free(line);
            free(args);
            continue;
        }
        
        // external command execution for non-builtins
        execute(args);

        free(line);
        free(args);
    }
    printf("\n");
    return 0;
}