#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#define BUFFER_SIZE 4096
extern char **environ;
extern int errno;

int isRedirect(char* token){
    if(strncmp(token,"2>>",3)==0 ) return 5;
    else if(strncmp(token,">>",2)==0 ) return 4;
    else if(strncmp(token,"2>",2)==0 ) return 3;
    else if(strncmp(token,">",1)==0 ) return 2;
    else if(strncmp(token,"<",1)==0 ) return 1;
    else return 0;
}
        
void shellExit(char* exitCode, unsigned childStatus){
    if (exitCode[0] != '\0') 
        exit( atoi(exitCode) );
    
    else if (WIFSIGNALED(childStatus) ) 
        exit(128 + WTERMSIG(childStatus));
    
    else  
        exit(WEXITSTATUS(childStatus));
}

void shellCd(char* path){
    if ( path[0] == '\0') 
        chdir(getenv("HOME"));
    else 
        chdir(path);
}

void shellPwd(char* buf){
    memset(buf,0,BUFFER_SIZE);        
    getcwd(buf, BUFFER_SIZE);
    printf("Current Working Directory: %s\n", buf);
}

int main(int argc, char** argv){
    char *line = NULL;
    size_t linsize = 0; // have getline allocate a buffer for us
    
    // Decide usage as interpreter or command line
    FILE *fp;
    if (argv[1] != NULL){ 
        fp = fopen(argv[1], "r");
        if (fp == NULL){
            perror("Error with opening file as interpeter");
            exit(1);
        }
    }
    else {
        fp = stdin;
    }

    char buf[BUFFER_SIZE]; // general buffer
    char *token;           // strok tokens
    const char s[2] = " "; // delimiter

    unsigned childStatus;
    struct rusage ru;
    
    pid_t cpid;
    char arguments[100][BUFFER_SIZE];   // limit of 100 arguments
    int argCount;                       // keep track of arguments[] size
    char redir[3][BUFFER_SIZE];         // max possible 3 redirections
    int redirCount;                     // keep track of redir[] size

    int isComment; // 1 if yes, 0 if no
    struct timeval t_start, t_end;

    while( getline(&line, &linsize, fp)>=0 ){
        if (line[strlen(line)-1]=='\n') line[strlen(line)-1]='\0'; 
        if (line[0] =='\0') continue; // ignore empty lines

        isComment   =  0;
        argCount    =  0;
        redirCount  =  0;
        memset(arguments, 0, sizeof arguments);
        memset(redir, 0, sizeof redir);

        token = strtok(line,s);
    
        while(token != NULL && !isComment ){
        //    printf("token: %s\n", token);
        //    printf("argcount: %d\n", argCount);

            if(argCount==0 && strncmp(token,"#",1) == 0){
                memset(buf,0,BUFFER_SIZE);   
                isComment=1;
            }

            //  check for redirections
            if(isRedirect(token) != 0){
                strcpy(redir[redirCount], token);
                // printf("Redirected: %s\n", redir[redirCount]);
                redirCount++;
            }

            // check for arguments
            else {
                strcpy(arguments[argCount], token);
                // printf("Argument: %s\n", arguments[argCount]);
                argCount++;
            }  
            token = strtok(NULL,s);
        }

        if(isComment==1) continue;

        //check for built-in commands
        if(strcmp( arguments[0], "cd" )==0){
            shellCd(arguments[1]);
        }

        else if(strncmp( arguments[0], "pwd", 3 )==0){
            shellPwd(buf);
        }

        else if(strcmp( arguments[0], "exit" )==0){
            shellExit(arguments[1], childStatus);
        }

        //forking, redirecting, formatting arguments, execvp, statistics
        else {
            int fd;
            gettimeofday(&t_start, NULL);   // start realtimer time
            switch( cpid = fork() ){

                case -1:
                    perror("fork failed");
                    return -1;

                case 0:; // child case
                    fclose(fp); // Clean up fd table inherited
                    int i;
                    for (i=0; i<redirCount; i++) {
                        // for <filename 
                        if (isRedirect(redir[i]) == 1) {
                            if( (fd=open(redir[i]+1, O_RDONLY, 0666)) < 0){
                                fprintf(stderr, "Cannot open %s\n", redir[i]+1);
                                perror("ERROR");
                                exit(1);
                            }

                            if (dup2(fd, 0) < 0){
                                fprintf(stderr, "Cannot redirect %s to stdin\n", redir[i]+1);
                                perror("ERROR");
                                exit(1);
                            }
                            close(fd);  
                        } 

                        // for >filename
                        else if (isRedirect(redir[i]) == 2) {
                            if( (fd = open(redir[i]+1, O_CREAT|O_TRUNC|O_WRONLY, 0666)) < 0){
                                fprintf(stderr, "Cannot open %s\n", redir[i]+1);
                                perror("ERROR");
                                exit(1);
                            }
                            if(dup2(fd, 1) < 0){
                                fprintf(stderr, "Cannot redirect %s to stdout\n", redir[i]+1);
                                perror("ERROR");
                                exit(1);
                            }
                            close(fd);  
                        }
                        // for 2>filename
                        else if (isRedirect(redir[i]) == 3) {
                            if ( (fd = open(redir[i]+2, O_CREAT|O_TRUNC|O_WRONLY, 0666)) < 0){
                                fprintf(stderr, "Cannot open %s\n", redir[i]+2);
                                perror("ERROR");
                                exit(1);
                            }
                            if(dup2(fd, 2) < 0){
                                fprintf(stderr, "Cannot redirect %s to stderr\n", redir[i]+2);
                                perror("ERROR");
                                exit(1);
                            }  
                            close(fd);  
                        }
                        // for >>filename
                        else if (isRedirect(redir[i]) == 4) {
                            if( (fd = open(redir[i]+2, O_CREAT|O_APPEND|O_WRONLY, 0666)) < 0){
                                fprintf(stderr, "Cannot open %s\n", redir[i]+2);
                                perror("ERROR");
                                exit(1);
                            }
                            if(dup2(fd, 1) < 0){
                                fprintf(stderr, "Cannot redirect %s to stdout\n", redir[i]+2);
                                perror("ERROR");
                                exit(1);
                            } 
                            close(fd);  
                        }
                        // for 2>>filename
                        else if (isRedirect(redir[i]) == 5) {
                            if( (fd = open(redir[i]+2, O_CREAT|O_APPEND|O_WRONLY, 0666)) < 0){
                                fprintf(stderr, "Cannot open %s\n", redir[i]+2);
                                perror("ERROR");
                                exit(1);
                            }
                            if(dup2(fd, 2) < 0){
                                fprintf(stderr, "Cannot redirect %s to stderr\n", redir[i]+2);
                                perror("ERROR");
                                exit(1);
                            } 
                            close(fd);  
                        }
                        // Somehow fails every redirection case, despite being labelled as a redirection.
                        // This should never happen, but here for redundancy.
                        else{
                            fprintf(stderr, "Error Redirecting: %s\n", redir[i]);
                        }
                    }

                    // Now we will execute the command.
                    // Make a pointer array to our arguments, as required by the exec 'v' family
                    char * argPointers[100];
                    for (i=0; i<argCount; i++){
                        argPointers[i] = &arguments[i][0];
                    }

                    if ( execvp( arguments[0], argPointers) < 0 ){
                        fprintf(stderr, "Tried command : %s\n", arguments[0]);
                        perror("ERROR on exec");
                        exit(127);
                    }
                    break;

                default: // parent case
                    if (wait3(&childStatus, 0, &ru) == -1){
                        perror("Error with wait3");
                    }
                    else {
                        // //timer ends here
                        gettimeofday(&t_end, NULL);
                        if ( childStatus == 0 ) 
                            fprintf(stderr, "Child process %d exited normally", cpid);
                        else if ( WIFSIGNALED(childStatus))
                             fprintf(stderr, "Child process %d exited with signal %d", cpid,  WTERMSIG(childStatus));
                        else // then must return code != 0
                             fprintf(stderr, "Child process %d exited with exit code %d", cpid,  WEXITSTATUS(childStatus));

                        fprintf(stderr, "\nReal: %f (sec)   User: %f (sec)   System: %f (sec)\n\n",
                        (double) (t_end.tv_usec - t_start.tv_usec) / 1000000 + (double) (t_end.tv_sec - t_start.tv_sec),
                        (double) (ru.ru_utime.tv_usec) / 1000000 + (double) (ru.ru_utime.tv_sec),
                        (double) (ru.ru_stime.tv_usec) / 1000000 + (double) (ru.ru_stime.tv_sec) );   
                    }
                    break;
                }
        } 
        // end of parsing of this line, reset buffer and go to next line
        memset(buf,0,BUFFER_SIZE);
    }
    shellExit("\0", childStatus);
}