#ifndef _SEM_H
#define _SEM_H

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include "spinlock.h"
#define N_PROC 64

struct sem{
    int sem_count;
    sigset_t mask, oldmask;
    pid_t waitlist[N_PROC]; // arbitrary number for now
    //declare spinlock
    //volatile char *sem_lock
};

void sem_init(struct sem *s, int count);
// Initialize the semaphore *s with the initial count. Initialize
// any underlying data structures. sem_init should only be called
// once in the program (per semaphore). If called after the
// semaphore has been used, results are unpredictable.

// The pointer s is assumed to point within an established
// area of shared memory. This function does not allocate it!


int sem_try(struct sem *s);
// Attempt to perform the "P" operation (atomically decrement
// the semaphore). If this operation would block, return 0,
// otherwise return 1.

void sem_wait(struct sem *s);
// Perform the P operation, blocking until successful. See below
// about how blocking and waking are to be implemented.

void sem_inc(struct sem *s);
// Perform the V operation. If any other tasks were sleeping
// on this semaphore, wake them by sending a SIGUSR1 to their
// process id (which is not the same as the virtual processor number).
// If there are multiple sleepers (this would happen if multiple
// virtual processors attempt the P operation while the count is <1)
// then all must be sent the wakeup signal.

void handler(int sig);
// handle SIGUSR1
#endif