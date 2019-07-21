#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>


/*
** Compile and run this program, and make sure you get the 'aargh' error
** message. Fix it using a pthread mutex. The one command-line argument is
** the number of times to loop. Here are some suggested initial values, but
** you might have to tune them to your machine.
** Debian 8: 100000000
** Gouda: 10000000
** OS X: 100000
** You will need to compile your program with a "-lpthread" option.
*/

#define handle(p) ({ p; int errtmp = errno; if (errtmp != 0)  { perror(#p) ; exit(errtmp); } })


#define NUM_THREADS 2

int pthread_mutex_init(pthread_mutex_t * mutex, const pthread_mutexattr_t * attr);
int pthread_mutex_destory(pthread_mutex_t *mutex);

pthread_mutex_t lock;

int i;

void *foo (void *bar) {
    handle(printf("in a foo thread, ID %ld\n", (long) pthread_self()));

    handle(pthread_mutex_lock(&lock));

    for (i = 0; i < *((int *) bar); i++) {
        int tmp = i;
        
        if (tmp != i) {
            handle(printf ("aargh: %d != %d\n", tmp, i));
        }
    }
    handle(pthread_mutex_unlock(&lock));

    pthread_exit ((void *)pthread_self());
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        handle(printf("You must supply a numerical argument.\n"));
        exit(1);
    }

    int iterations = strtol(argv[1], NULL, 10);
    assert(errno == 0);
    assert(iterations > 0);

    pthread_t threads[NUM_THREADS];
    int j;
    for (j = 0; j < NUM_THREADS; j++) {
        if (pthread_create(&threads[j], NULL, foo, (void *) &iterations)) {
            perror ("pthread_create");
            return (1);
        }
    }

    
    handle(pthread_mutex_init(&lock, NULL)); /* changed to dynamic init instead of static PTHREAD_MUTEX_INITIALIZER, advice from Luke*/
    for (j = 0; j < NUM_THREADS; j++) {
        void *status;
        
        if (pthread_join (threads[j], &status)) {
            perror ("pthread_join");
            return (1);
        }
        
        handle(printf("joined a foo thread, number %ld\n", (long) status));
    }
    handle(pthread_mutex_destroy(&lock));
    return (0);
}