#ifndef BUILTINS_H
#define BUILTINS_H

// is_builtin()  – returns non-zero if cmd is a built-in
// run_builtin() – runs it; returns 1 if handled, 0 if not a built-in

int is_builtin(char *cmd);
int run_builtin(char **argv) ;


#endif