#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include "syscall.h"

int eye2eh(int i, char *buffer, int buffersize, int base);

int status, exit_status;

void handler(int sigval){
    syscall(wait(&status));

    if(WIFSIGNALED(status)){
        exit_status = WTERMSIG(status);
    }

    if(sigval == SIGCHLD){
        WRITESTRING("SIGCHLD signal recieved, exited with status: ");
        WRITEINT(exit_status, 4);
        WRITESTRING("\n");
        exit(0);
    }
}

int main(){
    pid_t fork_pid;

    syscall(fork_pid=fork());


    struct sigaction action;
    action.sa_handler = handler;
    sigemptyset (&action.sa_mask);
    action.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    syscall(sigaction(SIGCHLD, &action, NULL));

    
    if(fork_pid==0){
        syscall(execl("./child", "child", NULL));
    }

    
    int i;
    for(i = 0; i < 5; i++){
        syscall(printf("Going to SIGSTOP %ld \n", (long)fork_pid));
        syscall(kill(fork_pid, SIGSTOP));
        syscall(sleep(2));
        syscall(printf("Going to SIGCONT %ld \n", (long)fork_pid));
        syscall(kill(fork_pid, SIGCONT));
        syscall(sleep(2));
    }

    syscall(kill(fork_pid, SIGINT));

    syscall(pause());

    exit(0);
}