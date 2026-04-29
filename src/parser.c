#include <stdlib.h> // malloc, free, exit
#include <string.h> // strtok, strcmp
#include <stdio.h>  // fprintf for error handling

#include "parser.h"

/*
    ----- Input tokenization and redirection parsing -----

    Splits a raw input line into an argv array, then scans for < and >
    redirections. If found, the filename gets recorded and the operator
    is pulled out of argv so the executor doesn't see it.

    Empty input leaves argv[0] as NULL, caller should skip it.
*/

char *in_file = NULL;
char *out_file = NULL;

char **tokenize(char *line) {
    in_file = NULL;
    out_file = NULL;

    char **argv = malloc(sizeof(char *) * MAX_ARGS);
    if (!argv) {
        fprintf(stderr, "Allocation error\n");
        exit(EXIT_FAILURE);
    }

    int argc = 0;
    char *token = strtok(line, " \t\r\n");

    while (token != NULL && argc < MAX_ARGS - 1) { 
        argv[argc++] = token;
        token = strtok(NULL, " \t\r\n"); 
    }

    argv[argc] = NULL; // null-terminate the argv array

    // detect any redirection
    for (int i = 0; argv[i] != NULL; i++) {

        if (strcmp(argv[i], "<") == 0) {
            if (argv[i + 1] == NULL) {
                fprintf(stderr, "syntax error: expected filename after '<'\n");
                break;
            }
            in_file = argv[i + 1]; // store input file for redirection
            argv[i] = NULL;        // drop the operator from argv
            break;
        }
        if (strcmp(argv[i], ">") == 0) {
            if (argv[i + 1] == NULL) {
                fprintf(stderr, "syntax error: expected filename after '>'\n");
                break;
            }
            out_file = argv[i + 1]; 
            argv[i] = NULL; 
            break;
        }
    }

    return argv;
}

