/*
 * Jason Louie (V00804645)
 * CSC 360 Assignment 1
 * PMan.c supplied with Makefile and Readme.txt
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

//Structure "proc" that holds a pid supplemented by A1-hint-official.c
typedef struct {
	pid_t pid;
}proc;

//prints the layout structure from bglist command
void print_processes(int list_size, proc *proc_list){
	char *cwd;
	char buff[4096+1];
	cwd = getcwd(buff,4096+1);
	int i;
	for(i = 0; i < list_size; i++){
		printf("%d:\t%s\n",(proc_list[i]).pid,cwd);
	}
	printf("Total background jobs:\t%d\n", list_size);
}

int main(){
	pid_t pid;
	char cont = 'y';
	proc *proc_list[35];
	int list_size = 0;
	
	//cmd entails the command to trigger which execution, typically 'bg' or 'bglist' while acmd are the arguments like 'cat foo.txt'
	while(cont == 'y'){
		char *cmd = (char *)malloc(sizeof(char));
       	 	char acmd[100];

		printf("Enter a command, 'n' to exit: ");
	        scanf("%s", cmd);
        	fgets(acmd, 100, stdin);

	        char *array[100];
		//keyword 'n' exits the program
		if(strcmp(cmd, "n") == 0){
			printf("PMan terminated\n");
			exit(1);
		}
        	char *token = (char *)malloc(sizeof(char));
	        token = strtok(acmd, " ");
        	int i = 0;
	        while(token != NULL){
        	        array[i] = token;
               		i++;
	                token = strtok(NULL, "\n ");
       		}
	        char *temp = NULL;
        	array[i] = temp;
		int k_pid = atoi(array[0]);
 
		//three kill commands with their respective signals
		if(strcmp(cmd, "bgkill") == 0){
			if(kill((pid_t)k_pid, SIGTERM) != 0){
				 printf("Process kill failed");
			}
		}else if(strcmp(cmd, "bgstart") == 0){
			if(kill((pid_t)k_pid, SIGCONT) != 0){
				printf("Process kill failed");
			}
		}else if (strcmp(cmd, "bgstop") == 0){
			if(kill((pid_t)k_pid, SIGSTOP) != 0){
				printf("Process kill failed");
			}
		//bg command
		}else if(strcmp(cmd, "bg") == 0){
			pid = fork();
			if(pid > 0){
				wait(NULL);
			}else if(pid == 0){
				execvp(array[0], array);			
				printf("error in exec!\n");
			}else{
				printf("error in fork");
				exit(1);
			}
			//create and add a pointer to proc_list, afterwards free the pointer due to malloc
			proc *ptr = (proc *)malloc(sizeof(proc));
			(*ptr).pid = pid;
                        proc_list[list_size] = ptr;
                        list_size++;
			free(ptr);
	
		//bglist command that calls upon print_processes
		}else if(strcmp(cmd, "bglist") == 0){
			print_processes(list_size,*proc_list);
		//pstat command, not implemented
		}else if(strcmp(cmd, "pstat") == 0){
			char path[2024];
			FILE *fp = popen("/proc/[pid]/stat(status)", "r");	
		}else{
			printf("used unknown command\n");
		}
		free(cmd);
		free(token);
	}
	return 0;
}
