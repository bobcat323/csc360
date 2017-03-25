/*
 * Jason Louie (V00804645)
 * CSC 360 Assignment 3
 * diskinfo.c supplied with Makefile, readme.txt, disklist.c, diskget.c, diskput.c
 */
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ROOT_OFFSET 0x2600
#define FAT_OFFSET 0x200

//print the OS name
void printOSName(char* address, char* osName){
	osName = address + 0x3;
	printf("OS Name: %s\n", osName);
}
	
//print the disk label
void printLabel(char* address, char* label){
	label = address + ROOT_OFFSET + 0x60;
	printf("Label of the disk: %s\n", label);
}

//print disk dize
void printDiskSize(struct stat myStat, int size){
	size = myStat.st_size;
	printf("Total size of the disk: %d bytes\n", size);
}
//print the number of files
void printNumFiles(char* address){
	int cluster = address[ROOT_OFFSET];
	int count;

	count = -1;
	while(address[cluster] != 0){
		if(address[cluster + 0x1a] == 0){
			count++;
			cluster += 0x20;
			continue;
		}
		cluster += 0x20;
	}
	printf("The number of files in the root directory (not including subdirectories): %d\n", count);
}

//return the value in hexadecimal based on the even or odd number index
int fatPack(char* address, int index){
	int fourbits, eightbits, position1, position2, hexValue;
	char* finalVal = malloc(sizeof(char)*12);
	if(index % 2 == 1){//odd
		eightbits = 1 + floor((3*index)/2);
		fourbits = floor((3*index)/2);
		position1 = address[FAT_OFFSET + eightbits] & 0xFF; 
		position2 = address[FAT_OFFSET + fourbits] & 0xF;
		
		//concat the two values together
		char val[8], secondVal[4];
		sprintf(val, "%02x", position1);
		sprintf(secondVal, "%x", position2);
		strcat(val, secondVal);
		strcpy(finalVal, val);
	}else{//even
		fourbits = 1 + floor((3*index)/2);
		eightbits = floor((3 * index)/2);

		position1 = address[FAT_OFFSET + fourbits] & 0x0F;
		position2 = address[FAT_OFFSET + eightbits];
		
		//concat the two values together
                char val[4], secondVal[8];
                sprintf(val, "%02x", position1);
                sprintf(secondVal, "%x", position2);
		strcat(val, secondVal);
		strcpy(finalVal, val);
	}
        hexValue = (int)strtol(finalVal,NULL,16);
	return hexValue;
}

//print the free disk space (bytes)
void printFreeSize(char* address, int files, struct stat myStat){
	int i, freeSectors, hexValue;
	int firstLogicSec = address[ROOT_OFFSET + 0x1a];
	
	for(i = 2; i <= myStat.st_size/512; i++){
		hexValue = fatPack(address, i);
		if(hexValue == 0x000){
			freeSectors++;
		}
	}

	printf("Free size of the disk: %d bytes\n", freeSectors * 512);
}

//print the number of FAT copies
void printNumFAT(char* address, int numFAT){
	numFAT = address[0x10];
        printf("Number of FAT copies: %d\n", numFAT);
}

//print how many sectors per FAT
void printSecPFat(char* address, int sectors){
        sectors = address[0x16];
	printf("Sectors per FAT: %d\n", sectors);
}

int main(int argc, char* argv[]){
	int fileCheck, fStatCheck, numFAT,fileSize, sectors, files, size;
	struct stat myStat;
	char* address;
	char* osName = malloc(sizeof(char)*8);
	char* label = malloc(sizeof(char)*8);

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
	
        printOSName(address, osName); 
	printLabel(address, label);
	printDiskSize(myStat, size);
	printFreeSize(address, files, myStat);
	printf("==============\n");
	printNumFiles(address);
        printf("==============\n");
	printNumFAT(address, numFAT);	
	printSecPFat(address, sectors);

	free(osName);
	free(label);
	munmap(address, myStat.st_size);
	close(fileCheck);

	return 0;
}
