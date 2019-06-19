#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/msg.h>



#define handle(p) ({ p; int errtmp = errno; if (errtmp != 0)  { perror(#p) ; exit(errtmp); } }) // From Luke & Dr. Beaty


void handler(int sigval){
    if(sigval == SIGUSR1){
        char str[] = "SIGUSR1 signal recieved\n";
        write(1, str, sizeof(str));
    }
    else if(sigval == SIGUSR2){

    }
    return;
}

int main(){
    /*
    pid_t fork_pid;
    int status, exit_status;

    handle(fork_pid=fork());

    if(fork_pid==0){
        handle(execl("./child", "child", NULL));
    }
    */


    struct sigaction action;
    action.sa_handler = handler;
    sigemptyset (&action.sa_mask);
    action.sa_flags = SA_RESTART;
    assert (sigaction (SIGUSR1, &action, NULL) == 0);

    exit(0);
}


