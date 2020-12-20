#ifndef _FIFO_H
#define _FIFO_H

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "spinlock.h"
#include "sem.h"

#define MYFIFO_BUFSIZ 4096

volatile struct fifo{
    unsigned long buf[MYFIFO_BUFSIZ];
    int next_write,next_read;
    int item_count;
    struct sem sem_write;
    struct sem sem_read;
    struct sem sem_write_buf;
    struct sem sem_read_buf;
} fifo;

void fifo_init(struct fifo *f);
// Initialize the shared memory FIFO *f including any
// required underlying initializations (such as calling sem_init)
// The FIFO will have a fifo length of MYFIFO_BUFSIZ elements,
// which should be a static #define in fifo.h (a value of 4K is
// reasonable).

void fifo_wr(struct fifo *f, unsigned long d);
// Enqueue the data word d into the FIFO, blocking
// unless and until the FIFO has room to accept it.
// Use the semaphore primitives to accomplish blocking and waking.
// Writing to the FIFO shall cause any and all processes that
// had been blocked on the "empty" condition to wake up.

unsigned long fifo_rd(struct fifo *f);
// Dequeue the next data word from the FIFO and return it.
// Block unless and until there are available words
// queued in the FIFO. Reading from the FIFO shall cause
// any and all processes that had been blocked on the "full"
// condition to wake up.

void fifo_debug();

#endif