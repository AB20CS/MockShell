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

int interp_rec(const struct cmd *c);

void return_from_command(int *fd) {
    if ((*fd) != STDOUT_FILENO) {
        close(*fd);
    }
    *fd = STDOUT_FILENO; // restore original stdout
}

int echo(int fd, const struct cmd *c) {
    write(fd, c->data.echo.arg, strlen(c->data.echo.arg));
    
    return_from_command(&fd);
    return 0;
}

int forx(int fd, const struct cmd *c) {
    pid_t pid;
    int status, exit_status;
    
    int pipefd[2];
    pipe(pipefd);

    pid = fork();

    if (pid == 0) { // child process
        close(pipefd[0]); // close reading end in child
        dup2(pipefd[1], STDOUT_FILENO); // send stdout to pipe

        close(pipefd[1]);

        int exec_ret;
        if ((exec_ret = execvp(c->data.forx.pathname, c->data.forx.argv)) == -1) {
            perror("exec");
            fd = STDOUT_FILENO; // restore original stdout
            exit(127);
        }
    }
    else { // parent process
        wait(&status);

        close(pipefd[1]); // close writing end in parent
        
        char buf[1024];
        int nbytes;
        while ((nbytes = read(pipefd[0], buf, sizeof(buf))) > 0) {
            write(fd, buf, nbytes);
        }

        if (WIFEXITED(status)) {
            return_from_command(&fd);
            return WEXITSTATUS(status);
        }
        else if (WIFSIGNALED(status)) {
            return_from_command(&fd);
            return 128 + WTERMSIG(status);
        }
    }
}

int list(int fd, const struct cmd *c) {
    
    int return_val = 0;
    int fd_list = fd;
    for (int i = 0; i < c->data.list.n; i++) {

        if ( c->data.list.cmds[i].redir_stdout != NULL) {
            int fd_tmp = creat(c->data.list.cmds[i].redir_stdout, 0666);
            printf("truncate %s (%d)\n", c->data.list.cmds[i].redir_stdout, fd_tmp);
            if (fd_tmp == -1) {
                perror("truncate file");
                exit(1);
            }
            close(fd_tmp);
        }
        // save stdout_redir file currently in command and change it to fd
        bool req_redir = false;
        if (c->data.list.cmds[i].redir_stdout == NULL &&  c->redir_stdout != NULL) {
            c->data.list.cmds[i].redir_stdout = malloc(1024);
            strcpy(c->data.list.cmds[i].redir_stdout, c->redir_stdout);
            req_redir = true;
        }
        
        
        if (c->data.list.cmds[i].type == LIST)
            return_val = interp(&(c->data.list.cmds[i]));
        else
            return_val = interp_rec(&(c->data.list.cmds[i]));

        // restore command's redirection info
        if (req_redir) {
            free(c->data.list.cmds[i].redir_stdout);
            c->data.list.cmds[i].redir_stdout = NULL;
        }

        // if command or descendent is killed by SIGINT
        if (return_val == 128 + SIGINT) {
            return return_val;
        }

        return_from_command(&fd);
        fd = fd_list;
    }
    return_from_command(&fd);
    return return_val;
}

int interp_rec(const struct cmd *c)
{
    int fd;
    if (c->redir_stdout == NULL) {
        printf("set fd to stdout\n");
        fd = STDOUT_FILENO;
    }
    else {
        fd = open(c->redir_stdout, O_WRONLY|O_CREAT|O_APPEND, 0666);
        printf("REC redirect stdout to %s (%d)\n", c->redir_stdout, fd);
        if (fd == -1) {
            perror("open redirection file");
            exit(1);
        }
    }
    
    // ECHO
    if (c->type == ECHO) {
        printf("\tcalling echo\n");
        return echo(fd, c);
    } 

    // FORX
    else if (c->type == FORX) {
        printf("\tcalling forx\n");
        return forx(fd, c);
    }

    // LIST
    else if (c->type == LIST) {
        printf("\tcalling list\n");
        return list(fd, c);
    }
}

int interp(const struct cmd *c)
{
    int fd;
    if (c->redir_stdout == NULL) {
        // printf("write to stdout\n");
        fd = STDOUT_FILENO;
    }
    else {
        fd = creat(c->redir_stdout, 0666);
        printf("redirect stdout to %s (%d)\n", c->redir_stdout, fd);
        if (fd == -1) {
            perror("open redirection file");
            exit(1);
        }
    }
    
    // ECHO
    if (c->type == ECHO) {
        return echo(fd, c);
    } 

    // FORX
    else if (c->type == FORX) {
        return forx(fd, c);
    }

    // LIST
    else if (c->type == LIST) {
        return list(fd, c);
    }
}
