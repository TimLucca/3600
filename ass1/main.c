#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    const char *args[] = {"./counter", argv[1], NULL};
    pid_t pid;
    int status, exit_status;

    if((pid=fork()) < 0)
        perror("fork failed");
    
    if(pid==0){
        execvp(args[0], args);
    }

    if((pid=wait(&status)) == -1){
        perror("wait failed");
        exit(2);
    }

    if(WIFEXITED(status)){
        exit_status = WEXITSTATUS(status);
        assert(exit_status = atoi(argv[1]));
        printf("Process %d exited with status: %d \n", pid, exit_status);
    }
    
    return 0;
}