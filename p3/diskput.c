/*
 * Jason Louie (V00804645)
 * CSC 360 Assignment 3
 * diskput.c supplied with Makefile, readme.txt, diskinfo.c, disklist.c, diskget.c
 */
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ROOT_OFFSET 0x2600
#define FAT_OFFSET 0x200

//a structure to hold all the information 
struct File{
	char name[9];
	char extension[4];
	char attribute;
	char reserved[3];
	char createTime[2];//hour, minute format
	char createDate[3];//year, month, day format
	char recentAccessDate[3];
	char ignore[2];
	char writeTime[2];
	char writeDate[3];
	int firstCluster;
	int size;
};

//return a pointer to a character array of the file name
char* fillName(char* argv){
	char* ptr;
	char* token;
	char name[9];
	int length = sizeof(name);

	token = strtok(argv, ".");
	strcpy(name, token);
	//pad with extra spaces
	for(; length < length - 1; length++){
		name[length] = ' ';
	}
	
	name[sizeof(name) - 1] = '\0';
	ptr = name;
	return ptr;
}

//return a pointer to a character array of the extension
char* fillExtension(char* argv){
	char* ptr;
        char* token;
        char name[4];
        int length = sizeof(name);
	token = &argv[sizeof(argv) - 4];
	strcpy(name, token);
	ptr = name;
	return ptr;
}

//get the hexadecimal value based on the index if it is even or odd
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

//general function to search for free space (search FAT table and root directory)
int searchSpace(char* address, int entry, int interval){
	int index, hexValue, randval;
	index = 0;
	if(entry == 0x200){
		hexValue = fatPack(address, index);
		while(hexValue != 0x000){
                	
			index++;
        	        hexValue = fatPack(address, index);
        	}
		entry = index + 0x200;
	}else{
		while(address[entry] != 0x000){
			entry += interval;
		}		
	}
	return entry;
}

//returns a struct with respective information about the file
struct File populateStruct(char* address, struct stat myStat2, char* argv){
	struct File argStat;
	time_t t;
	struct tm tm;
	int entry;
	
	strcpy(argStat.name, fillName(argv));
        strcpy(argStat.extension, fillExtension(argv));
        argStat.attribute = '1';  //read only bit on, subdirectory bit off to indicate a file
        argStat.reserved[0] = '0';
        argStat.reserved[1] = '0';

        //get current time and date and store in struct
        t = time(NULL);
        tm = *localtime(&t);

        argStat.createTime[0] = (char)tm.tm_hour;
        argStat.createTime[1] = (char)tm.tm_min;

        argStat.createDate[0] = (char)tm.tm_year;
        argStat.createDate[1] = (char)tm.tm_mon;
        argStat.createDate[2] = (char)tm.tm_mday;

        argStat.recentAccessDate[0] = (char)tm.tm_year;;
        argStat.recentAccessDate[1] = (char)tm.tm_mon;
        argStat.recentAccessDate[2] = (char)tm.tm_mday;

        argStat.ignore[0] = '0';
        argStat.ignore[1] = '0';

        argStat.writeTime[0] = (char)tm.tm_hour;
        argStat.writeTime[1] = (char)tm.tm_min;

        argStat.writeDate[0] = (char)tm.tm_year;
        argStat.writeDate[1] = (char)tm.tm_mon;
        argStat.writeDate[2] = (char)tm.tm_mday;
	//search for first available entry in FAT table
        entry = searchSpace(address, FAT_OFFSET, 1);
        argStat.firstCluster = entry - FAT_OFFSET;
        argStat.size = myStat2.st_size;

	return argStat;
}

//creates a char array holding all the information
char* infoToString(struct File argStat){
	char* ptr;
	char info[500];
	char convert[5];
	ptr = info;
	
	strcat(info, argStat.name);
	strcat(info, argStat.extension);	
	strcat(info, &argStat.attribute);
	strcat(info, argStat.reserved);
        strcat(info, argStat.createTime);
        strcat(info, argStat.createDate);
        strcat(info, argStat.recentAccessDate);
        strcat(info, argStat.ignore);
        strcat(info, argStat.writeTime);
        strcat(info, argStat.writeDate);

	sprintf(convert, "%d", argStat.firstCluster);
        strcat(info, convert);
	//empty the array
	memset(convert, ' ', 5);
	sprintf(convert, "%d", argStat.size);
      	strcat(info, convert);

	return ptr;
}

int main(int argc, char* argv[]){
	int i, fileCheck1, fileCheck2, fStatCheck1, fStatCheck2, entry, physSector, nextFreeSec;
	struct stat myStat1;//disk2.IMA
	struct stat myStat2;//FOO.TXT
	struct File argStat;//FOO.TXT
	char* address;	
	FILE* file1, *file2;//file1 = disk2.IMA, file2 = argv[2] = FOO.TXT

	file1 = fopen(argv[1], "r+");
	fileCheck1 = open(argv[1], O_RDONLY);
	if(fileCheck1 == -1){
		fclose(file1);
		perror("open/argv[1]");
		exit(1);
	}
	
	fStatCheck1 = fstat(fileCheck1, &myStat1);
        if(fStatCheck1 == -1){
                perror("fstat1");
                close(fileCheck1);
		fclose(file1);
                exit(1);
        }

	address = mmap(NULL, myStat1.st_size, PROT_READ, MAP_SHARED, fileCheck1, 0);
	if(address == MAP_FAILED){
		perror("mmap/address");
		close(fileCheck1);
		fclose(file1);
		exit(1);
	}
	
	//open file in root
	file2 = fopen(argv[2], "r");
	fileCheck2 = open(argv[2], O_RDONLY);
	
	if(fileCheck2 == -1){
		printf("File not found.");
		close(fileCheck1);
		fclose(file1);
		exit(1);
	}

	fStatCheck2 = fstat(fileCheck2, &myStat2);
        if(fStatCheck2 == -1){
                perror("fstat2");
                close(fileCheck1);
		close(fileCheck2);
		fclose(file1);
                fclose(file2);
                exit(1);
        }
	//fill the structure with file info
	argStat = populateStruct(address, myStat2, argv[2]);
	
	physSector = 31 + argStat.firstCluster;
	//printf("%x", physSector*512);
	//search/verify for free space in root directory and place all info into sector
	entry = searchSpace(address, ROOT_OFFSET, 0x20);
	//check if there is enough free space in root directory
	for(i = entry; i< 32; i++){
		if(address[entry] != 0){
			printf("Not enough free space in the disk image: root directory\n");
			close(fileCheck1);
			close(fileCheck2);
			fclose(file1);
			fclose(file2);
			exit(1);
		}
	}
	
	//create root directory info into a char array
	char* cptr;
	cptr = infoToString(argStat);
	//write info to root directory
	fseek(file1, ROOT_OFFSET, SEEK_SET);
	fwrite(cptr, 1,argStat.size,file1);
	int* intptr;
	int j = 0xFFF;
	intptr = &j;
	//update FAT: change first logical cluster value to 0xFFF to indicate end of file
	if(myStat2.st_size > 512){
		nextFreeSec = searchSpace(address, entry, 1);
		argStat.firstCluster = nextFreeSec;
		fseek(file1, nextFreeSec, SEEK_SET);
		fwrite(intptr, 1, sizeof(j), file1);
	}else{
		fseek(file1, argStat.firstCluster + 0x200, SEEK_SET);
		fwrite(intptr, 1, sizeof(j), file1);
	}

	//read the contents of the file (until \0 character, end of a string)
	char content[myStat2.st_size];
	fread(content, myStat2.st_size, 1, file2);	
        //write content of foo into data sector
	fseek(file1, physSector,SEEK_SET);
	fwrite(content, myStat2.st_size, 1, file1);

	munmap(address, myStat1.st_size);
	close(fileCheck1);
        close(fileCheck2);
	fclose(file1);
	fclose(file2);
	return 0;
}
