#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include "byos.h"
// Find out what other #include's you need! (E.g., see man pages.)

int interp(const struct cmd *c)
{
    int fd;
    if (c->redir_stdout == NULL) {
        printf("write to stdout\n");
        fd = STDOUT_FILENO;
    }
    else {
        fd = creat(c->redir_stdout, 0666);
        printf("redirect stdout to %s (%d)\n", c->redir_stdout, fd);
        if (fd == -1) {
            perror("open redirection file");
            return 1;
        }
    }
    
    // ECHO
    if (c->type == ECHO) {
        write(fd, c->data.echo.arg, strlen(c->data.echo.arg));
        return 0;
    } 

    // FORX
    else if (c->type == FORX) {
        pid_t pid;
        int status, exit_status;
        
        int pipefd[2];
        pipe(pipefd);

        pid = fork();

        if (pid == 0) { // child process
            close(pipefd[0]); // close reading end in child
            dup2(pipefd[1], STDOUT_FILENO); // send stdout to pipe

            // close(pipefd[1]);

            int exec_ret;
            if ((exec_ret = execvp(c->data.forx.pathname, c->data.forx.argv)) == -1) {
                perror("exec");
                exit(127);
            }
        }
        else { // parent process
            wait(&status);

            close(pipefd[1]); // close writing end in parent
            
            char buf[1024];
            int nbytes;
            while ((nbytes = read(pipefd[0], buf, sizeof(buf))) > 0) {
                printf("%d\n", fd);
                write(fd, buf, nbytes);
            }

            if (WIFEXITED(status)) {
                return WEXITSTATUS(status);
            }
            else if (WIFSIGNALED(status)) {
                return 128 + WTERMSIG(status);
            }
        }
    }

    // LIST
    else if (c->type == LIST) {
        int return_val = 0;
        for (int i = 0; i < c->data.list.n; i++) {
            // save stdout_redir file currently in command and change it to fd
            bool req_redir = false;
            if (c->data.list.cmds[i].redir_stdout == NULL &&  c->redir_stdout != NULL) {
                c->data.list.cmds[i].redir_stdout = malloc(1024);
                strcpy(c->data.list.cmds[i].redir_stdout, c->redir_stdout);
                req_redir = true;
            }
            
            // call command
            return_val = interp(&(c->data.list.cmds[i]));
            
            // restore command's redirection info
            if (req_redir) {
                free(c->data.list.cmds[i].redir_stdout);
                c->data.list.cmds[i].redir_stdout = NULL;
            }

            // if command or descendent is killed by SIGINT
            if (return_val == 128 + SIGINT) {
                return return_val;
            }
        }
        return return_val;
    }
}
