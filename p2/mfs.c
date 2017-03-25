/*
 * Jason Louie (V00804645)
 * CSC 360 Assignment 2 
 * mfs.c supplied with Design Document, Makefile, readme.txt, flow.txt, flow2.txt, flow3.txt 
 * program structure credited to Yongjun Xu
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#define MAXFLOW 50

typedef struct _flow
{
    float arrivalTime ;
    float transTime ;
    int priority ;
    int id ;
} flow;


flow flowList[MAXFLOW];   // parse input in an array of flow
flow *queueList[MAXFLOW];  // store waiting flows while transmission pipe is occupied.
pthread_t thrList[MAXFLOW]; // each thread executes one flow
pthread_mutex_t trans_mtx = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t trans_cvar = PTHREAD_COND_INITIALIZER;  
int queueMin, queueMax, usingID, numFlow;
bool available;
time_t startTime, endTime, begin, end;

//helper method to keep track of queue
void printQueue(){
	int i;
	printf("-----QUEUELIST----- %d\n", queueMin);
	for(i = queueMin; i < queueMax; i++){
		printf("%d\n",(*queueList[i]).id);
	}
	printf("-----QUEUELIST-----%d\n",queueMax);
}

//basic swap method for rearranging the queue
void swap(int a, int b){
	flow *temp = queueList[a];
	queueList[a] = queueList[b];
	queueList[b] = temp;
}

//sort queue according to rules: priority, arrival, transmission, first appearing
void sort(){
	int i, j;
	
	for(i = queueMin; i < queueMax; i++){
		for(j = i+1; j < queueMax; j++){
			if((*queueList[i]).priority > (*queueList[j]).priority){
				swap(i,j);
			}else if((*queueList[i]).priority == (*queueList[j]).priority){
				if((*queueList[i]).arrivalTime > (*queueList[j]).arrivalTime){
					swap(i,j);
				}else if((*queueList[i]).arrivalTime == (*queueList[j]).arrivalTime){
					if((*queueList[i]).transTime > (*queueList[j]).transTime){	
						swap(i,j);
					}else if((*queueList[i]).transTime == (*queueList[j]).transTime){
						if((*queueList[i]).id > (*queueList[j]).id){
							swap(i,j);
						}
					}
				}
			}
		}
	}
}

//organize queue and wait if transmission is full
void requestPipe(flow *item) {    
	pthread_mutex_lock(&trans_mtx);	
  	
    //if transmission pipe is available AND empty queue
	if(available && queueMax == 0){
		usingID = (*item).id;
		available = false;
		pthread_mutex_unlock(&trans_mtx);
		return;
	}
    //add item in queue, sort the queue according rules
	queueList[queueMax] = item;
	queueMax++;
	sort();
	
	if(!available){
		printf("Flow %2d waits for the finish of flow %2d\n", (*item).id,usingID);
    	}
    // wait till pipe to be available and be at the top of the queue
	while(!available && (*item).id != (*queueList[queueMin]).id){	
		pthread_cond_wait(&trans_cvar, &trans_mtx);
	}
	printf("Out of wait with id: %d\n", (*item).id);
	
	available = false;
	usingID = (*item).id;
	queueMin = queueMin +1;

	pthread_mutex_unlock(&trans_mtx);
}

//reopens the transmission pipe after the flow is done
void releasePipe() {
	pthread_mutex_lock(&trans_mtx);
	available = true;
	pthread_cond_broadcast(&trans_cvar);
	pthread_mutex_unlock(&trans_mtx);
}

// entry point for each thread created
void *thrFunction(void *flowItem) {
	flow *item = (flow *)flowItem;

    	// wait for arrival
    	usleep((*item).arrivalTime * 1000000);

	startTime = time(NULL);
   	printf("Flow %2d arrives: arrival time (%.2f), transmission time (%.1f), priority (%2d)\n", (*item).id, (*item).arrivalTime, (*item).transTime, (*item).priority);
	end = time(NULL);
	float  startDuration = (time_t)difftime(end, begin);
    	requestPipe(item);
	
	printf("Flow %2d starts its transmission at time %.2f.\n", (*item).id, startDuration);

    	// sleep for transmission time
    	usleep((*item).transTime * 100000);
	
    	releasePipe(item);

	endTime = time(NULL);
	int flowDuration = (time_t)difftime(endTime, startTime);
	printf("Flow %2d finishes its transmission at time %d\n", (*item).id, flowDuration);
	pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
	if(argc !=2){
		perror("Too many arguments!\n");
		return -1;
	}
	
	int i, j, error;
	char line[500], fileName[50];
	char *token = (char *)malloc(sizeof(char));
	char ar[50];
	queueMin = queueMax = 0;
	available = true;
	begin = time(NULL);
	
    // file handling
	FILE *fp = fopen(argv[1], "r");
	if(!fp){
		perror("File open error\n");
		return -1;
	}

    // read number of flows
	fgets(ar, 15, fp);
	numFlow = atoi(ar);

	for(i = 0; i < numFlow; i++){
		fgets(line,15,fp);

		token = strtok(line, ":");
		flowList[i].id = atoi(token);
		
                token = strtok(NULL, ",");
               	flowList[i].arrivalTime = atof(token);

                token = strtok(NULL, ",");
                flowList[i].transTime = atof(token);

                token = strtok(NULL, "\n");
                flowList[i].priority = atoi(token);
	}
    //release file descriptor 
	fclose(fp);
	
    //create n threads where n is number of flows 	
	for(i = 0; i < numFlow; i++){
		error = pthread_create(&thrList[i], NULL, thrFunction, (void *)&flowList[i]);
		if(error != 0){
			printf("Can't create thread! Exiting\n");
			exit(0);
		}
	}
    	
    // wait for all threads to terminate
    	for(j = 0; j < numFlow; j++){
                error = pthread_join(thrList[j],NULL);
		if(error != 0){
			printf("Can't joine thread! Exiting\n");
                        exit(0);
		}
        }
	
    // destroy mutex & condition variable
	pthread_mutex_destroy(&trans_mtx);
	pthread_cond_destroy(&trans_cvar);
	return 0;
}
