#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

// globals
int matched = 0;

void uppercase(char *word){
    int i;
    for (i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
}

void handler(int sig){
    fprintf(stderr, "(wordsearch) Process %d has recieved signal %d\n", getpid(), sig);

    fprintf(stderr, "Matched %d words\n", matched);

    exit(0);
}

int main(int argc, char *argv[]){
    char *word;
    FILE *fp;
    char **words = malloc((1000000 * sizeof(char *)));
    ssize_t read;
    ssize_t readcmp;
    size_t len = 0;

    signal(SIGPIPE, handler);

    int i;
    for (i=0; i<1000000; i++) // define the size of the string pointed at
        words[i] = malloc((15)*sizeof(char));

    //open provided input list of dictionary words
    if ( ( fp  =  fopen(argv[1], "r") ) == NULL){
        fprintf(stderr, "Error opening file %s\n", argv[1]);
        perror("ERROR"); 
    }

    //read dictionary one line at a time
    i=0;
    while ((read =  getline(&word, &len, fp) != -1)){
        
        int length = strlen(word);

        // remove \n char, update length
        if (word[length-1] == 10){
            word[length-1] = 0;
            length = strlen(word);
        }

        // only add words under this condition
        if( length<3 || length>15) continue;

        uppercase(word);
        strcpy(words[i], word);
        //fprintf(stderr, "String copied to words[%d] : %s\n", i, words[i]);
        i++;
    }
    
    fclose(fp);

    int dictLen = i;
    for(i=0; i<dictLen; i++){
        // fprintf(stderr, "In dict[%d]:%s\n", i, words[i]);
    }

    fprintf(stderr, "ACCEPTED %d WORDS\n", dictLen);

    int j;
    while( ( readcmp = getline(&word, &len, stdin) ) != -1){
        int length = strlen(word);
        if (word[length-1] == 10) 
            word[length-1] = 0;

        uppercase(word); 
        for (j = 0; j < dictLen; j++){
            if (strcmp(word, words[j]) == 0){
                matched++;
                fprintf(stdout, "Match: %s\n", word);
                break;
            }
        }
    }

    //read a line at a time from standard input and determine if that potential word matches your word list
    fprintf(stderr, "Matched %d words\n", matched);
    exit(0);
}
