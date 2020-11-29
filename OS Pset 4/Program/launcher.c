#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h> 
#include <signal.h>
#include <sys/signal.h>
#include <sys/resource.h>
#include <sys/wait.h>

// initiate two pipes in the main parent process
// PIPE 1 [ R wordsearch <------- wordgen W ] 
// PIPE 2 [ R pager <----------wordsearch W ]

int waitChild(){
    int childStatus = 0;
    pid_t cpid;

    if ( ( cpid = wait(&childStatus) ) == -1){
        perror("Error with wait");
        return -1;
    }

    else {
        if ( childStatus == 0 ) 
            fprintf(stderr, "Child process %d exited normally\n", cpid);
        else if ( WIFSIGNALED(childStatus))
            fprintf(stderr, "Child process %d exited with signal %d\n", cpid, WTERMSIG(childStatus));
        else // then must return code != 0
            fprintf(stderr, "Child process %d exited with exit code %d\n", cpid, WEXITSTATUS(childStatus));
    }

    return 0;
}

int makeChild(char *childArgs[], int pp1[], int pp2[], int readEnd, int writeEnd){
    pid_t pid;

    if( (pid = fork()) == 0){
        fprintf(stderr,"In child process %d (%s)\n", getpid(), childArgs[0]);

        // piping stage
        if (dup2(readEnd,0) < 0){
            fprintf(stderr, "Error redirecting wordgen to read end of pipe\n");    
        }

        if (dup2(writeEnd,1) < 0){
            fprintf(stderr, "Error redirecting wordgen to write end of pipe\n");    
        }

        close(pp1[0]); close(pp1[1]); close(pp2[0]); close(pp2[1]);

        if ( execvp(childArgs[0], childArgs) < 0){
            fprintf(stderr, "Tried execing : %s\n", childArgs[0]);
            perror("execvp");
        }

        return -1;
    }

    else if ( pid==-1 ) {
        fprintf(stderr, "Error making child");
        perror("fork");
    }
}

int main(int argc, char *argv[]){
    char *dictionary = "./words.txt";
    int pp1[2];
    int pp2[2];
    char buf[4096];

    //open pipe 1
    if(pipe(pp1) < 0){
        fprintf(stderr, "Could not create pipe 1\n");
        perror("pipe");
    }

    //open pipe 2
    if(pipe(pp2) < 0){
        fprintf(stderr, "Could not create pipe 2\n");
        perror("pipe");
    }
    
    char * wordgenArgs[3] = {"./wordgen", argv[1], NULL};
    makeChild( wordgenArgs, pp1, pp2, 0, pp1[1] );

    char *wordsearchArgs[3] = {"./wordsearch", dictionary, NULL};
    makeChild(wordsearchArgs, pp1, pp2, pp1[0], pp2[1]);

    char *pagerArgs[2] = {"./pager", NULL};
    makeChild(pagerArgs, pp1, pp2, pp2[0], 1);
    
    // close all pipes for parent since we're done spawning children
    close(pp1[0]); close(pp1[1]); close(pp2[0]); close(pp2[1]);

    // recieve all the zombies
    waitChild(); waitChild(); waitChild();
    fprintf(stderr, "Recieved all zombie processes\n");
    fprintf(stderr, "Parent process is exiting...\n");
    return 0;
}
