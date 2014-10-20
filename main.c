//Blady Martinez and Jonah Fidel 
//Whole project coded collaboratively 
//10/19/14
//prof. Sommers 
//COSC 301 proj 02 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>

//method declarations 
char *** splitUp ( char * commands);
char ** tokenifyNew (char *value, char *delimiter);

//used for ** arrays 
int arraylen (char **liststr){
	int track = 0; 
	char *temp = liststr[track];
	while (temp != NULL){
		track++;
		temp = liststr[track];
	}
	return track;
}

//removes all text after # symbol 
char * checkComments(char * checkC){
	char * temp = strdup(checkC);
	for (int i = 0; i< strlen(temp);i++){
		if(temp[i]== '#'){
			temp[i] = '\0';
			break;
		}
	}
	return temp;
}

//splits on delimeter (either ";"" or " \t\n" in this case)
char ** tokenifyNew (char *value, char *delimiter) {
	char scopy [strlen(value)+1];
	for (int i = 0; i< strlen(value) + 1; i++){
		scopy[i] = value[i];
	}
	int track = 0; 
	char **arr = malloc(sizeof(char*)*(strlen(scopy)+2));
	char *token = NULL;
	char *temp;
	token = strtok(scopy, delimiter);
	while (token != NULL){
		//printf(" %s\n", token);
		temp = strdup(token);
		arr[track]= temp;
		track++;
		token = strtok(NULL, delimiter);
	}
	arr[track] = NULL;	
	return arr;
}

void free_tokens(char **tokens) {
    int i = 0;
    while (tokens[i] != NULL) {
        free(tokens[i]); // free each string
        i++;
    }
    free(tokens); // then free the array
}

//splits up input on semicolons, then splits tokens on whitespace, returning a pointer to the array of pointers
char ***splitUp (char * commands) {
	int a = strlen(commands)-1;
	if(commands[a] == '\n'){ 
		commands[a] = '\0'; 
	} 
	char ** listInputs = tokenifyNew(commands, ";");
	//print_tokens(listInputs);
	int total = arraylen(listInputs);
	char ***finalList = malloc(sizeof(char**) * (total+1));
	for (int b = 0; b < total; b++){
		finalList[b] = tokenifyNew(listInputs[b], " \n\t");
	}
	finalList[total] = NULL;
	free_tokens(listInputs);
	return finalList;
}

//runs processes sequentially 
void runSeq(char*** commandsRun){
	pid_t newPid = fork(); 
	if (newPid < 0){
		printf("Fork has failed =(\n");
	}
	else if (newPid == 0){
		if (strcmp(commandsRun[newPid][0],"exit") != 0){
			if (execv(commandsRun[newPid][0],commandsRun[newPid]) < 0){
				fprintf(stderr, "execv did not execute\n");
			}
		}
	}
	int currProcess = 0; 
	wait(&currProcess);
}

//runs processes in parallel 
int runPar(char*** commandsRun){
	int b = 0;
	int rv = 1;  
	int * status = NULL; 
	while(commandsRun[b] != NULL){
		if (strcmp(commandsRun[b][0], "exit") == 0){
				printf("exit called\n");
				rv = 2; 
		}
		else if (strcmp(commandsRun[b][0], "mode") == 0){
			if (arraylen(commandsRun[b]) == 1){
					printf("Program is running in Parallel Mode.\n");
			}
			else if ((strcmp(commandsRun[b][1], "p") == 0) || (strcmp(commandsRun[b][1], "parallel") == 0)){
				if (rv != 2){
					rv = 1; 
				}
				printf("set to run in parallel mode\n");
			}
			else if ((strcmp(commandsRun[b][1], "s") == 0) || (strcmp(commandsRun[b][1], "sequential") == 0)){
				if (rv != 2){
					rv = 0; 
				}
				printf("set to run in sequential mode\n");
			}
		}
		else {
			pid_t newPid = fork(); 
			if (newPid < 0){
				printf("Fork has failed =(\n");
			}
			else if (newPid == 0){
				if (execv(commandsRun[b][0],commandsRun[b]) < 0){
					fprintf(stderr, "execv did not execute\n");
				}
			}
		}
		waitpid(-1, status, 0); //functionality of parallel seems off, should be handeled here 
		b++; 
	}
	return rv; 
}

int main(int argc, char **argv) {
	int mode = 0; // sequential mode = 0, parallel mode = 1, exit = 2 
	int nextMode = 0; 
	while(1){
		if (mode == 2) {
			break; 
		}
		printf("%s", "> "); //prompt
		fflush(stdout);  
		char buffer[1024];
	    fgets(buffer, 1024, stdin);  // process current command line in buffer
	    char *input = checkComments(buffer);
		char *** splitInputs = splitUp(input);
		int i = 0;
		if (mode == 1){
			nextMode = runPar(splitInputs); //runPar returns 0, 1 or 2 to determine the next mode 
		}
		while (splitInputs[i] != NULL && (mode == 0)){
			if (strcmp(splitInputs[i][0], "exit") == 0){
				printf("exit called\n");
				nextMode = 2; 
			}
			else if (strcmp(splitInputs[i][0], "mode") == 0){
				if (arraylen(splitInputs[i]) == 1){
					if (mode == 0){
						printf("Program is running in Sequential Mode.\n");
					}
					else if (mode == 1){
						printf("Program is running in Parallel Mode.\n");
					}
				}
				else if ((strcmp(splitInputs[i][1], "p") == 0) || (strcmp(splitInputs[i][1], "parallel") == 0)){
					nextMode = 1; 
					printf("set to run in parallel mode\n");
				}
				else if ((strcmp(splitInputs[i][1], "s") == 0) || (strcmp(splitInputs[i][1], "sequential") == 0)){
					nextMode = 0; 
					printf("set to run in sequential mode\n");
				}
				
			}
			else {
				if (mode == 0) {
					runSeq(&splitInputs[i]);
				}
			}
			i++;	
		}
		mode = nextMode;
	}
    return 0;
}

