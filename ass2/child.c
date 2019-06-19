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

#define handle(p) ({ p; int errtmp = errno; if (errtmp != 0)  { perror(#p) ; exit(errtmp); } }) /* From Luke & Dr. Beaty */

int kill(pid_t pid, int sig);

int main(){
    pid_t parent_pid;
    
    handle(parent_pid=getppid());

    handle(kill(parent_pid, SIGUSR1));

    handle(kill(parent_pid, SIGUSR2));

    int i;
    for(i = 0; i < 3; i++)
        handle(kill(parent_pid, SIGWINCH));

    exit(0);
}