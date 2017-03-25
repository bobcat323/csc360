/*
 * Jason Louie (V00804645)
 * CSC 360 Assignment 3
 * disklist.c supplemented with Makefile, readme.txt, diskinfo.c, diskget.c, diskput.c
*/

#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define ROOT_OFFSET 0x2600
#define FAT_OFFSET 0x200

//determine if the file is a subdirectory or file
char getType(char* address, int entry){
	int fType = address[0x0b + entry] & 0x10;
	if(fType == 1){
		return 'D';
	}else{
		return 'F';
	}	
}

//take the high and low bits to compute a file size (bytes)
int getSize(char* address, int entry){
	int signedlow = address[entry + 0x1c];
	int signedhigh = address[entry + 0x1d];        

	//to unsigned int
	unsigned int low = (unsigned int)signedlow & 0xFF;
	unsigned int high = (unsigned int)signedhigh & 0xFF;
	
	int fileSize = (high<<8)|(low);
	
	return fileSize;
}

//outputs the file name with extension
void printName(char* address, int entry){
	char* fileName = address + entry;
	char* token;	
	char string[13], name[9], extension[4];
	
	strcpy(string, fileName);
	strncpy(name, string, 8);
	name[8] = '\0';
	token = strtok(name, " ");
	
	strcpy(extension, &string[strlen(string) - 3]);
	extension[3] = '\0';
	printf("%s.%s ",token, extension);
}

//output the date and time
void printDateTime(char* address, int entry){
	int bits;
	
	int lowDateCreated = address[entry + 0x11];
	int highDateCreated = address[entry + 0x10];
	int lowTimeCreated = address[entry + 0x0f];
	int highTimeCreated = address[entry + 0x0e];

	unsigned int lowD = (unsigned int)lowDateCreated & 0xFF;
	unsigned int highD = (unsigned int)highDateCreated & 0xFF;
	unsigned int lowT = (unsigned int)lowTimeCreated & 0xFF;
        unsigned int highT = (unsigned int)highTimeCreated & 0xFF;

	//concat lowD and highD
	char valD[16], lattervalD[8];
	sprintf(valD, "%x", lowD);
	sprintf(lattervalD, "%x", highD);
	strcat(valD, lattervalD);
	
	//concat lowT and highT
	char valT[16], lattervalT[8];
        sprintf(valT, "%x", lowT);
        sprintf(lattervalT, "%x", highT);
        strcat(valT, lattervalT);

	//convert hex into decimal number
	char *ptrD = valD;
	int decimalD = strtol(ptrD, NULL,16);

	char *ptrT = valT;
	int decimalT = strtol(ptrT, NULL, 16);

	//get year
	bits = decimalD >> 9;
	int year = 1980 + bits;
	printf("%d-",year);	
	//get month
	bits = decimalD >> 5;
	int month = bits & 0xF;
	printf("%d-", month);
	//get day	
	int day = decimalD & 0x1F;
	printf("%d ", day);

	//get hour
	int hour = decimalT >> 11;
	printf("%d:", hour);
	//get minute
	bits = decimalT >> 5;
	int minute = bits & 0x3F;
	printf("%02d", minute);
}

int main(int argc, char* argv[]){
	int fileCheck, fStatCheck, fileSize, nextEntry;
	char fileType;
	struct stat myStat;
	char* address;
	char* fileName;
	fileCheck = open(argv[1], O_RDONLY);
	if(fileCheck == -1){
		perror("open");
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
		perror("mmap");
		close(fileCheck);
		exit(1);
	}

	nextEntry = ROOT_OFFSET;
	while(address[nextEntry] != 0){	
		//ensure it is in root directory
		if(address[nextEntry + 0x1a] == 0){
			nextEntry+= 0x20;
			continue;
		}
		//F/D
	        printf("%c ", getType(address, nextEntry));
		//get file size
		printf("%d ", getSize(address, nextEntry));
		//file name
		printName(address, nextEntry);
		//file creation
		printDateTime(address, nextEntry);
		printf("\n");
	
		nextEntry += 0x20;
	}	

	munmap(address, myStat.st_size);
	close(fileCheck);
	return 0;
}
