#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>


#define handle(p) ({ p; int errtmp = errno; if (errtmp != 0)  { perror(#p) ; exit(errtmp); } }) /* From Luke & Dr. Beaty */


void handler(int sigval){
    if(sigval == SIGUSR1){
        char str[] = "SIGUSR1 signal recieved\n";
        write(1, str, sizeof(str));
    }
    else if(sigval == SIGUSR2){
        char str[] = "SIGUSR2 signal recieved\n";
        write(1, str, sizeof(str));
    }
    else if(sigval == SIGWINCH){
        char str[] = "SIGWINCH signal recieved\n";
        write(1, str, sizeof(str));
    }
    return;
}

int main(){
    
    pid_t fork_pid;
    int status, exit_status;

    handle(fork_pid=fork());


    struct sigaction action;
    action.sa_handler = handler;
    sigemptyset (&action.sa_mask);
    action.sa_flags = SA_RESTART;
    handle(sigaction(SIGUSR1, &action, NULL));
    handle(sigaction(SIGUSR2, &action, NULL));
    handle(sigaction(SIGWINCH, &action, NULL));

    if(fork_pid==0){
        handle(execl("./child", "child", NULL));
    }
    
    handle(fork_pid=wait(&status));

    if(WIFEXITED(status)){
        exit_status = WEXITSTATUS(status);
        assert(exit_status == 0);
    }

    exit(0);
}


