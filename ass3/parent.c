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

int status, exit_status, fork_pid;

void handler(int sigval){
    syscall(fork_pid=wait(&status));

    if(WIFEXITED(status)){
        exit_status = WEXITSTATUS(status);
        assert(exit_status == 0);
    }

    if(sigval == SIGCHLD){
        char str[] = "SIGCHLD signal recieved, exited with status: ";
        WRITESTRING(str);
        WRITEINT(exit_status, 2);
        WRITESTRING("\n");
        exit(0);
    }
}

int main(){

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