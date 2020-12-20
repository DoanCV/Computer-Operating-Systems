#include "spinlock.h"
#include <stdio.h>
#include "tas.h"
#include "sem.h"

void sem_init(struct sem *s, int count){
	s->sem_count = count;
	//Initialize any underlying data structures
	
    // Initialize mask ???
    // sigemptyset (&mask);
    // sigaddset (&mask, SIGUSR1);

    // Initialize waitlist pids
    int i;
    for (i = 0; i < N_PROC; i++){
        s->waitlist[i] = 0;
    }
}


int sem_try(struct sem *s){
    // Attempt to perform the "P" operation (atomically decrement
    // the semaphore). If this operation would block, return 0,
    // otherwise return 1.

    // "TRY", which attempts the P operation, but if the semaphore is not positive,
    // returns immediately with a failure code.

    if(s->sem_count > 0){
        sem_wait(s);
        return 0;
    } else {
        // FAILURE CODE
        //exit(FAILURE);
        return 1;
    }
    
}    

void sem_wait(struct sem *s){
// *** This will use the TAS assembly atomic decrease of variable
// which probably means we need to use our previous spinlock


// The P operation checks to see if the semaphore is positive. If it is not, the caller blocks.
// Otherwise, the semaphore is decremented by 1. This check and decrement is
// atomic. The P operation is also sometimes called "wait" or "down" or "dec"

    //To block the task in sem_wait, you
	// will use the sigsuspend system call, which has the useful property that it puts your process to sleep AND changes
	// the blocked signals mask atomically. The task sleeps until any signal is delivered to it. Then another task which
	// performs sem_inc will wake up the sleeping process at a later time, using SIGUSR1

	//int sigsuspend(const sigset_t *mask);
	
    if(s->sem_count > 0){
        s->sem_count--;
        return 1;
    } else {
        // using sigsuspend
        // Wait for a signal to arrive. 
        // sigprocmask(SIG_BLOCK, &mask, &oldmask);
        // while(!usr_interrupt) {
        //     sigsuspend(&oldmask);
        // }
        // sigprocmask(SIG_UNBLOCK, &mask, NULL);
        
        sigsuspend(&(s->mask));

        sem_inc(s); 
        return 0;
        
    }    
    
}

void sem_inc(struct sem *s){
    // The V operation increments the semaphore by 1. If the semaphore value is now positive, any
    // sleeping tasks are awakened. V is also called "inc" , "up" or "post".

    s->sem_count++;
    if(s->sem_count > 0){
        int i;
        for(i = 0; i < N_PROC; i++) {
            //SIGUSR1 -> pid
            //signal(SIGUSR1, handler);
            waitlist[i] = fork();
            if (waitlist[i] == -1){
                perror("fork");
            }
            else {
                // Keep sleeping until interupted
                while(1){
                    kill(waitlist[i] , SIGUSR1);
                    sleep(1);
                }
            }
        }
    }
}
// Perform the V operation. If any other tasks were sleeping
// on this semaphore, wake them by sending a SIGUSR1 to their
// process id (which is not the same as the virtual processor number).
// If there are multiple sleepers (this would happen if multiple
// virtual processors attempt the P operation while the count is <1)
// then all must be sent the wakeup signal.

void handler(int sig){
    fprintf(stderr, "Process %d has recieved signal %d\n", getpid(), sig);

    // Handle user interrupt

    exit(0);
}
