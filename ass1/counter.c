#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>


int main(int argc, char *argv[]){
    long n, child, parent;
    char *endptr;

    n = strtol(argv[1], &endptr, 10);
    if(*endptr != '\0'){
        if(fprintf(stderr, "bad input: %s", argv[1]) < 0){
            int errtemp = errno;
            perror("print failed");
            exit(errtemp);
        }
    }

    if(n==LONG_MAX){
        int errtemp = errno;
        perror("overflow");
        exit(errtemp);
    }
    else if(n==LONG_MIN){
        int errtemp = errno;
        perror("underflow");
        exit(errtemp);
    }

    if((child=(long)getpid()) < 0){
        int errtemp = errno;
        perror("getpid failed");
        exit(errtemp);
    }
    if((parent=(long)getppid()) < 0){
        int errtemp = errno;
        perror("getppid failed");
        exit(errtemp);
    }

    if(printf("Parent PID: %ld \n", parent) < 0){
        int errtemp = errno;
        perror("printf failed");
        exit(errtemp);
    }

    if(printf("Child PID: %ld \n", child) < 0){
        int errtemp = errno;
        perror("printf failed");
        exit(errtemp);
    }

    int i;
    for(i = 1; i <= n; i++){
        if(printf("Process: %ld %d \n", child, i) < 0){
            int errtemp = errno;
            perror("printf failed");
            exit(errtemp);
        }
    }

    exit(n);
}