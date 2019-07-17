#include "syscall.h"
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>


int main(){
    long pid;
    syscall(pid = getpid());

    while(1){
        syscall(printf("Awake in %ld \n", pid));
        syscall(sleep(1));
    }
    exit(0);
}