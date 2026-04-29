#include <stdio.h>  // fgets, fprintf
#include <stdlib.h> // malloc, free, exit
#include <string.h> // strcspn for trimming newline

#include "input.h"

/*
    ----- User input handling -----

    Reads a line from stdin, strips the trailing newline, and returns it.
    fgets() returning NULL means EOF (Ctrl+D) so the caller should treat
    that as a clean exit.

    Buffer is heap-allocated so the parser can modify it freely.
*/

char *read_line(void) {
    char *buffer = malloc(MAX_INPUT_SIZE);

    if (buffer == NULL) {
        fprintf(stderr, "Allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    // fgets is safer than scanf - fixed max size and it handles spaces
    if (fgets(buffer, MAX_INPUT_SIZE, stdin) == NULL) { 
        free(buffer);
        return NULL; // EOF or error
    }
    
    // strip the trailing newline (strcspn finds it and nulls it out)
    buffer[strcspn(buffer, "\n")] = '\0'; 
    return buffer;
}