#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    int n = atoi(argv[1]);

    printf("Parent PID: %ld \n", (long)getppid());
    printf("Child PID: %ld \n", (long)getpid());

    for(int i = 1; i <= n; i++){
        printf("Process: %ld %d \n", (long)getpid(), i);
    }

    exit(n);
}