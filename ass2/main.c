#include <stdio.h>
#include <errno.h>

#define handle(p) ({ p; int errtmp = errno; if (errtmp != 0)  { perror(#p) ; exit(errtmp) } }) // From Luke & Dr. Beaty


struct sigaction {

};


int main(){


    struct sigaction action;
    action.sa_handler = handler;
    sigemptyset (&action.sa_mask);
    action.sa_flags = SA_RESTART;
    assert (sigaction (SIGUSR1, &action, NULL) == 0);

    handle(pid = getppid());

}


