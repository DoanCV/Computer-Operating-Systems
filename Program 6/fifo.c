#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include "spinlock.h"
#include "tas.h"
#include "sem.h"
#include "fifo.h"

void fifo_debug(struct fifo *f){
    fprintf(stderr, 
        "\nnext_write: %d\n"
        "next_read: %d\n"
        "sem_write count: %d\n"
        "sem_read count: %d\n\n",
        f->next_write,
        f->next_read,
        f->sem_write.sem_count,
        f->sem_read.sem_count
    );
}

void fifo_init(struct fifo *f){

    // Circular buffer
    f->next_write = 0;
    f->next_read = 0;

    // Buffer semaphores
    sem_init(&f->sem_write_buf, 1);
    sem_init(&f->sem_read_buf, 1);

    // 0 meaning it is empty
    // 4096 meaning it is full
    sem_init(&f->sem_read, 0);

    // 0 meaning it is full
    // 4096 meaning it is empty
    sem_init(&f->sem_write, MYFIFO_BUFSIZ);
}

void fifo_wr(struct fifo *f, unsigned long d){

    // Get the green light to write
    sem_wait(&f->sem_write);

    // We are now APPROVED to write!!
    // Get in the buffer line
    sem_wait(&f->sem_write_buf);
    
    f->buf[f->next_write++] = d;
    fprintf(stderr, "%d wrote '%lu' @i=%d \n", getpid(), d,f->next_write-1);
    f->next_write %= MYFIFO_BUFSIZ;
    sem_inc(&f->sem_read); // notify the readers that there is smth to read
    
    sem_inc(&f->sem_write_buf);  // release the buffer lock!
}

unsigned long fifo_rd(struct fifo *f){
    unsigned long d;

    // Get the green light that there is smth to read
    sem_wait(&f->sem_read);
    
    // We are now approved to READ!!
    // Get in the buffer line
    sem_wait(&f->sem_read_buf);

    d = f->buf[f->next_read++];
    fprintf(stderr, "%d read %lu @i=%d\n", getpid(), d, f->next_read-1);
    f->next_read %= MYFIFO_BUFSIZ;
    sem_inc(&f->sem_write);    // Notify writers that space has been created

    sem_inc(&f->sem_read_buf); // release the buffer lock

    return d; 
}