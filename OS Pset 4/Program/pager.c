#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]){
  char *line = NULL;
  int lineCount = 0;
  size_t len = 0;
  ssize_t nread;
  
  FILE * ttyIn;
  FILE * ttyOut;

  if ( (ttyOut = fopen("/dev/tty", "w")) == NULL){
    fprintf(stderr, "Error opening /dev/tty for write with fopen()");
    perror("fopen");
  }

  if ( (ttyIn = fopen("/dev/tty", "r")) == NULL){
    fprintf(stderr, "Error opening /dev/tty for read with fopen()");
    perror("fopen");
  };

  while (( nread = getline(&line, &len, stdin)) != -1 ){
    lineCount++;
    fwrite(line, nread, 1, stdout);
    
    if (lineCount%23 == 0){
      char c;
      fprintf(ttyOut, "---Press RETURN for more---\n");

      while(1){
        c = fgetc(ttyIn);
        if (c==10) break;
        if (c==113 || c==81) exit(0);
      }

    }

  }
  
  fclose(ttyOut);
  fclose(ttyIn);
  return 0;

}
