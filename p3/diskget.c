/*
 * Jason Louie (V00804645)
 * CSC 360 Assignment 3
 * diskget.c supplied with Makefile, readme.txt, diskinfo.c, disklist.c diskput.c
 */
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define ROOT_OFFSET 0x2600
#define FAT_OFFSET 0x200

//verify that the file is in the current directory
bool checkName(char* address, int entry, char* argument){
        char* fileName = address + entry;
        char* token;
        char string[13], name[9], extension[4];
	char* file = malloc(sizeof(char)*14);

        strcpy(string, fileName);
        strncpy(name, string, 8);
	name[8] = '\0';

        token = strtok(name, " ");

        strcpy(extension, &string[strlen(string) - 3]);
	extension[3] = '\0';    
	strcat(file, token);
	strcat(file, ".");
	strcat(file, extension);
	file[13] = '\0';
	
	if(strcmp(file,argument) == 0){
		return true;
	}
	free(file);
	return false;
}

//get the file size of the file
int getFileSize(char* address, int entry){
	int signedlow = address[entry + 0x1c];
	int signedhigh = address[entry + 0x1d];
	
	unsigned int low = (unsigned int)signedlow & 0xFF;
	unsigned int high = (unsigned int)signedhigh & 0xFF;

	int size = (high<<8)|low;
	
	return size;
}

int main(int argc, char* argv[]){
	int fileCheck, fStatCheck, nextEntry, fileSize;
	struct stat myStat;
	char* address;
	char* dest;
	FILE* fp;

	fileCheck = open(argv[1], O_RDONLY);
	if(fileCheck == -1){
		perror("open/arg[1]");
		exit(1);
	}
	
	fStatCheck = fstat(fileCheck, &myStat);
	if(fStatCheck == -1){
		perror("fstat");
		close(fileCheck);
		exit(1);
	}
	
	address = mmap(NULL, myStat.st_size, PROT_READ, MAP_SHARED, fileCheck, 0);
	if(address == MAP_FAILED){
		perror("mmap/address");
		close(fileCheck);
		exit(1);
	}
	
	//search in root for filename	
	nextEntry = ROOT_OFFSET;
        while(address[nextEntry] != 0){
               	if(checkName(address, nextEntry, argv[2])){
			//get filesize if there is a match
			fileSize = getFileSize(address, nextEntry);
			break;
		}
           
                nextEntry += 0x20;
        }

	if(checkName(address, nextEntry, argv[2]) == false){
		perror("File not found");
		exit(1);
	}else{
		//get file size and related info
		int logsec = address[nextEntry +0x1a];
		int physsec = 31 + logsec;
		char* data = malloc(sizeof(char)*8);
		data = address + (physsec * 512);
		//open a file in current directory
		fp = fopen(argv[2], "w");
		//copy files from source -> destination,
		fwrite(data, 1, fileSize, fp);
		fclose(fp);
	}

	munmap(address, myStat.st_size);
	close(fileCheck);
	
	return 0;
}
