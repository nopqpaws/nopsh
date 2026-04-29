#ifndef PARSER_H
#define PARSER_H

#define MAX_ARGS 64

extern char *in_file;   // set by < redirection, consumed by executor
extern char *out_file;  // set by > redirection, consumed by executor

// tokenize() – split line into a null-terminated argv array, pulling out
//              any < or > redirections into in_file/out_file.
char **tokenize(char *line);

#endif