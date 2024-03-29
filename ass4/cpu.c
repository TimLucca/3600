#define _GNU_SOURCE

#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include "systemcall.h"
#include "eye2eh.c"
#include <time.h>

/*
This program does the following.
1) Creates handlers for two signals.
2) Create an idle process which will be executed when there is nothing
   else to do.
3) Create a send_signals process that sends a SIGALRM every so often.

The output looks approximately like:

[...]
Sending signal:  14 to process: 35050
Stopping: 35052
---- entering scheduler
Continuing idle: 35052
---- leaving scheduler
Sending signal:  14 to process: 35050
at the end of send_signals
Stopping: 35052
---- entering scheduler
Continuing idle: 35052
---- leaving scheduler
---- entering process_done
Timer died, cleaning up and killing everything
Terminated: 15

---------------------------------------------------------------------------
Add the following functionality.
1) Change the NUM_SECONDS to 20.

2) Take any number of arguments for executables and place each on the
   processes list with a state of NEW. The executables will not require
   arguments themselves.

3) When a SIGALRM arrives, scheduler() will be called; it currently simply
   restarts the idle process. Instead, do the following.
   a) Update the PCB for the process that was interrupted including the
      number of context switches and interrupts it had and changing its
      state from RUNNING to READY.
   b) If there are any NEW processes on processes list, change its state to
      RUNNING, and fork() and execl() it.
   c) If there are no NEW processes, round robin the processes in the
      processes queue that are READY. If no process is READY in the
      list, execute the idle process.

4) When a SIGCHLD arrives notifying that a child has exited, process_done() is
   called.
   a) Add the printing of the information in the PCB including the number
      of times it was interrupted, the number of times it was context
      switched (this may be fewer than the interrupts if a process
      becomes the only non-idle process in the ready queue), and the total
      system time the process took.
   b) Change the state to TERMINATED.
   c) Restart the idle process to use the rest of the time slice.
*/

#define NUM_SECONDS 20
#define ever ;;

enum STATE { NEW, RUNNING, WAITING, READY, TERMINATED, EMPTY };

struct PCB {
    enum STATE state;
    const char *name;   /* name of the executable */
    int pid;            /* process id from fork(); */
    int ppid;           /* parent process id */
    int interrupts;     /* number of times interrupted */
    int switches;       /* may be < interrupts */
    time_t started;        /* the time this process started */
};

typedef enum { false, true } bool;

int PROCESSTABLESIZE;
struct PCB processes[10];

struct PCB idle;
struct PCB *running;

int next_run = 0; /* Thanks Luke */

int new = 1;
int sys_time;
int timer;
struct sigaction alarm_handler;
struct sigaction child_handler;

void bad(int signum) {
    WRITESTRING("bad signal: ");
    WRITEINT(signum, 4);
    WRITESTRING("\n");
}

/* cdecl> declare ISV as array 32 of pointer to function(int) returning void */
void(*ISV[32])(int) = {
/*       00   01   02   03   04   05   06   07   08   09 */
/*  0 */ bad, bad, bad, bad, bad, bad, bad, bad, bad, bad,
/* 10 */ bad, bad, bad, bad, bad, bad, bad, bad, bad, bad,
/* 20 */ bad, bad, bad, bad, bad, bad, bad, bad, bad, bad,
/* 30 */ bad, bad
};

void ISR (int signum) {
    if (signum != SIGCHLD) {
        kill (running->pid, SIGSTOP);
        WRITESTRING("Stopping: ");
        WRITEINT(running->pid, 6);
        WRITESTRING("\n");
    }

    ISV[signum](signum);
}

void send_signals(int signal, int pid, int interval, int number) {
    int i;
    for(i = 1; i <= number; i++) {
        sleep(interval);
        WRITESTRING("Sending signal: ");
        WRITEINT(signal, 4);
        WRITESTRING(" to process: ");
        WRITEINT(pid, 6);
        WRITESTRING("\n");
        systemcall(kill(pid, signal));
    }

    WRITESTRING("At the end of send_signals\n");
}

void create_handler(int signum, struct sigaction action, void(*handler)(int)) {
    action.sa_handler = handler;

    if (signum == SIGCHLD) {
        action.sa_flags = SA_NOCLDSTOP | SA_RESTART;
    } else {
        action.sa_flags =  SA_RESTART;
    }

    systemcall(sigemptyset(&action.sa_mask));
    systemcall(sigaction(signum, &action, NULL));
}

void check_new() {
    int i;
    for(i=0; i<PROCESSTABLESIZE; i++){
        if(processes[i].state == NEW){
            running->switches++;
            running = &processes[i];
            WRITESTRING ("Switching to ");
            WRITESTRING(running->name);
            WRITESTRING ("\n");
            running->state = RUNNING;
            systemcall(running->ppid = getpid());
            systemcall(running->pid = fork());
            running->started = time(NULL);
            if(running->pid==0){
                systemcall(execl(running->name, running->name, NULL));
            }
            return;            
        }
    }
    WRITESTRING("No more new processes.\n");
    new = 0;
    return;
}

int check_ready(int start, int end){
    int i;
    for(i=start; i<end; i++){
        if(processes[i].state == READY){
            if(running->pid != processes[i].pid){
                running->switches++;
                running = &processes[i];
                WRITESTRING ("Switching to ");
            }
            else{
                WRITESTRING ("Continuing ");
            }
            WRITESTRING(running->name);
            WRITESTRING ("\n");
            running->state = RUNNING;
            systemcall(kill(running->pid, SIGCONT));
            next_run=i+1;
            return 1;
        }
    }
    return 0;
}

void scheduler (int signum) {
    WRITESTRING("---- entering scheduler\n");
    assert(signum == SIGALRM);

    running->interrupts++;
    running->state = READY;
    
    if(new){
        check_new();
        return;
    }

    if(check_ready(next_run, PROCESSTABLESIZE)){
        return;
    }

    if(check_ready(0, next_run)){
        return;
    }

    WRITESTRING ("No processes ready, continuing idle: ");
    WRITEINT (idle.pid, 6);
    WRITESTRING ("\n");
    running = &idle;
    idle.state = RUNNING;

    systemcall (kill (idle.pid, SIGCONT));

    WRITESTRING("---- leaving scheduler\n");
}

void process_done (int signum) {
    WRITESTRING("---- entering process_done\n");
    assert (signum == SIGCHLD);

    time_t end = time(NULL);

    long elapsed = (long)end - (long)running->started;

    WRITESTRING("Process ");
    WRITEINT(running->pid, 6);
    WRITESTRING(" has ended\n");
    WRITEINT(running->interrupts, 4);
    WRITESTRING(" interrupts, ");
    WRITEINT(running->switches, 4);
    WRITESTRING(" switches.\nTime elapsed: ");
    WRITEINT((int)elapsed, 6);
    WRITESTRING("\n");

    running->state = TERMINATED;

    running = &idle;

    WRITESTRING ("---- leaving process_done\n");
}

void boot()
{
    sys_time = 0;

    ISV[SIGALRM] = scheduler;
    ISV[SIGCHLD] = process_done;
    create_handler(SIGALRM, alarm_handler, ISR);
    create_handler(SIGCHLD, child_handler, ISR);

    assert((timer = fork()) != -1);
    if (timer == 0) {
        send_signals(SIGALRM, getppid(), 1, NUM_SECONDS);
        WRITESTRING ("Timer died, cleaning up and killing everything\n");
        systemcall(kill(0, SIGTERM));
        exit(0);
    }
}

void create_idle() {
    idle.state = READY;
    idle.name = "IDLE";
    idle.ppid = getpid();
    idle.interrupts = 0;
    idle.switches = 0;
    idle.started = time(NULL);

    assert((idle.pid = fork()) != -1);
    if (idle.pid == 0) {
        systemcall(pause());
    }
}

void initialize_processes(int argc, char **args){
    PROCESSTABLESIZE = argc-1;
    assert(PROCESSTABLESIZE<=10);
    int i;
    for(i=0; i<PROCESSTABLESIZE; i++){
        struct PCB tmp;
        tmp.name = args[i+1];
        tmp.state = NEW;
        tmp.switches = 0;
        tmp.interrupts = 0;
        processes[i] = tmp;
    }
}

int main(int argc, char **argv) {
    initialize_processes(argc, argv);
    boot();
    create_idle();
    running = &idle;

    for(ever) {
        pause();
    }
}