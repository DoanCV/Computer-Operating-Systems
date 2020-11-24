#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h> 
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAXCHAR 1000

void replace(char* str1, char* str2, struct stat sb, char *pageptr){
    int i;
    int j;
    int length = strlen(str1);

    for (i = 0; i < sb.st_size; i++){
        
        char* temp = &pageptr[i];
        if( strncmp( temp, str1, strlen(str1)) == 0 ){
            
            for( j=0 ; j<length; j++) {
                pageptr[i+j] = str2[j];
            }
        }
    }
}

int main(int argc, char *argv[]){
    
    char replaceMe[MAXCHAR];
    char newString[MAXCHAR];
    int fd;
    
    strcpy(replaceMe, argv[1]);
    //printf("String we will be replacing: %s\n", replaceMe);

    strcpy(newString, argv[2]);
    //printf("Replacement string: %s\n", newString);
   
    int fileIt;
    struct stat sb;
    for( fileIt=3; fileIt<argc ; fileIt++){
        //fprintf( stderr, "Opening file %s\n", argv[fileIt] );

        if( (fd = open(argv[fileIt], O_RDWR)) < 0){
            fprintf(stderr, "Could not open file: %s\n", argv[3]);
            perror("ERROR");
            
        } else {
        
            // Gather information before mapping
            lstat(argv[fileIt], &sb);
            int lastPageLength = sb.st_size%4096;
            int pages = lastPageLength>0 ? (sb.st_size/4096)+1 : sb.st_size/4096;

            //fprintf(stderr, "Total bytes in file: %ld \n", sb.st_size);
            //fprintf(stderr, "Number of pages: %d \n", pages);
            //fprintf(stderr, "Length of last page: %d \n", lastPageLength);


            // map file regions with mmap()
            // use MAP_SHARED flag, so the changes appear in the actual file 
            // read write permissions
            // read and replace for each given file
            // mysnc() to back the dirty pageframes into the original file
            char *pageptr;

            // Allocate the shared mapping
            pageptr = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

            if (pageptr == MAP_FAILED){
                fprintf(stderr, "Mmap command failed on file %s\n", argv[fileIt]);
                perror("ERROR");
            }

            replace(replaceMe, newString, sb, pageptr);
            
            // Asynchronous msync 
            if ( msync(pageptr, 4096, MS_SYNC) == -1){ 
                fprintf(stderr, "Asynchronous msync raised error on file %s\n", argv[fileIt]);
                perror("ERROR");
            }

            // Delete mappings
            if( (munmap(pageptr, 4096)) == -1){
                fprintf(stderr, "Failed to delete mappings on file %s\n", argv[fileIt]);
                perror("ERROR");
            }
            
            
            close(fd);
        }
    }

    return 0;
}

