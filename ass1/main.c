#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main() {

    pid_t fork_pid;
    int status, exit_status;

    if((fork_pid=fork()) < 0){
        int errtemp = errno;
        perror("fork failed");
        exit(errtemp);
    }
    
    if(fork_pid==0){
        execl("./counter", "counter", "5", NULL);
        int errtemp = errno;
        perror("exec failed");
        exit(errtemp);
    }

    if((fork_pid=wait(&status)) == -1){
        int errtemp = errno;
        perror("wait failed");
        exit(errtemp);
    }

    if(WIFEXITED(status)){
        exit_status = WEXITSTATUS(status);
        assert(exit_status == 5);
        if(printf("Process %d exited with status: %d \n", fork_pid, exit_status) < 0){
            int errtemp = errno;
            perror("printf failed");
            exit(errtemp);
        }
    }
    
    exit(0);
}