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

int main(){
    pid_t parent_pid;
    
    handle(parent_pid=getppid());

    handle(kill(parent_pid, SIGUSR1));


    exit(0);
}