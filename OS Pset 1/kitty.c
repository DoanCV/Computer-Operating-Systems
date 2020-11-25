/* kitty.c program written by Philip Blumin, Paul Cucchiara and Brian Doan */
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<ctype.h>
extern int errno;

//global variables
char buf[4096];

//function that copies; this will read and write an input and output files
int copy(int input_fd, int output_fd, char* s){
	int size; 
	int bytes = 0;
	int reads = 0;
	int writes = 0;
	int flag = 1; //used as a bool
	while((size = read(input_fd, buf, sizeof(buf))) > 0) {
		bytes = bytes + size; //increment number of bytes for each file
		reads++; 
		for (int i = 0; i < size; i++){
			//check to see if file is binary
			if(!(isprint(buf[i])||isspace(buf[i])) && flag){
				flag = 0; //make this false so that it never comes back here
				printf("WARNING: File %s is binary or has binary elements in it!\n", s);
			}
		}
		if(write(output_fd, buf, size) < 0){
			printf("Could not write %s\n", s);
			printf("Error Number %d\n", errno);
			exit(-1);
		}
		writes++;
	}
	reads++;
	printf("Name of of file concatenated: %s\n", s);
	printf("Number of bytes: %d\n", bytes);
    printf("Number of reads: %d\n", reads);
    printf("Number of writes: %d\n", writes);
	
	if (size < 0) {
		return -1;
	}
	return 0;
}

int main(int argc, char *argv[]) {
	int c; //for switch case
	char *outputfile; 
	int start = 1; //start of forloop
	int ofd = 1; //preset ofd to be stdout

	while((c = getopt(argc, argv, "o")) != -1)
		switch(c) {
			//case where we have -o
			case 'o':
			outputfile = argv[2];
			start = 3;
			ofd = open(outputfile,O_WRONLY|O_CREAT|O_TRUNC,0666);
			if(ofd < 0){
				printf("Output file not given\n");
				printf("Error Number %d\n", errno);
				perror("OH NO! ERROR");
			}
			break;
		}
		
	for(int i = start; i < argc; i++) {
		if(strcmp(argv[i],"-") == 0) {
			if(copy(0, ofd, "<standard input>") < 0) {
				//0, 1, 2 analogous to stdin, stdout, stderr
				fprintf(stderr, "could not concatenate standard input\n");
				printf("Error Number %d\n", errno);
				perror("OH NO! ERROR");
			} 
		} 
		else 
		{
			int fd = open(argv[i], O_RDONLY); //open file
			if(fd < 0){
				fprintf(stderr, "Can't open file %s for reading\n", argv[i]);
				printf("Error Number %d\n", errno);
				perror("OH NO! ERROR");
			} else {
				if(copy(fd, ofd, argv[i]) < 0){
					fprintf(stderr, "could not concatenate %s to standard output\n", argv[i]);
					printf("Error Number %d\n", errno);
					perror("OH NO! ERROR");
				}
				close(fd);
			}
		}		
	}	
	return 0;
}