#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>    // fork, execvp, chdir
#include <sys/wait.h>  // waitpid
#include <stdlib.h>    // exit for error handling
#include <fcntl.h>     // open for file redirection
#include <sys/stat.h>  // file permissions for redirection

#include "executor.h"
#include "parser.h"

/*
    ----- External command execution -----

    Runs anything that isn't a built-in. The flow is a standard fork/exec:

        1) fork() creates a child process
        2) child sets up any I/O redirection
        3) child calls execvp() and becomes the target program
        4) parent sits in waitpid() until it's done

    Redirection (<, >) works by opening the file and dup2()ing it onto
    stdin or stdout before execvp() is called. At that point, the program
    never knows the difference.

    Everything runs in the child, so external commands can't touch the
    shell's own state.
*/

void execute(char **argv) {

    pid_t pid = fork();

    if (pid == 0) { // child

        // input redirection
        if (in_file) {
            int fd = open(in_file, O_RDONLY);
            if (fd < 0) {
                perror("open");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        // output redirection
        if (out_file) {
            int fd = open(out_file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
            if (fd < 0) {
                perror("open");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        execvp(argv[0], argv); // replace child process with the command
        perror("execvp");
        exit(1);
    } else if (pid > 0) {      // parent
        waitpid(pid, NULL, 0);

        // reset redirection state for next command
        in_file = NULL;
        out_file = NULL;
    } else {
        perror("fork");
        return;
    }
}
