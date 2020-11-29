#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h> 

char randLetters() {
    return 'A' + (rand() % 26);
}

void randomWord(int maxLength, int minLength){
    int randLength = (rand() % (maxLength - minLength + 1)) + minLength;
    int flag = 0;
    int getLetter;

    while (flag < randLength){
        flag++;
        getLetter = randLetters();
        printf("%c", getLetter);
    }
    printf("\n");
}

int main(int argc, char**argv){
    
    srand(time(0));

    int maxLength = 15;
    int minLength = 3;
    int numWords; 

    if (argc == 1){
        numWords = 0;
    } 
    else {
        numWords = atoi(argv[1]);
    }

    if(numWords == 0){
        for (;;) {
            randomWord(maxLength, minLength);
        }
    } 
    else if (numWords > 0){
        
        int i;
        for (i = 0; i < numWords ; i++){
            randomWord(maxLength, minLength);
            //printf("%d\n", i);
        }
    } 
    else {
        fprintf(stderr, "ENTERED NEGATIVE NUMBER\n");
    }

    fprintf(stderr, "Finished generating %d candidate words\n", numWords);
    return 0;
}
