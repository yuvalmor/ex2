// Yuval Mor - 205380173

#include <stdbool.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#define SPACE " "
#define PROMPT ">"
#define SCAN_WITHOUT_SPACES "%[^\n]"
#define BACKGROUND "&"
#define FAILURE -1
#define NOT_EMPTY 1
#define EMPTY 0
#define MAX_JOBS 512
#define INITIATE_VALUE 0
#define IDENTICAL 0
#define SIZE_TO_ALLOCATE(X) (X+1)
#define ADD_CHAR(X) (X+1)
#define MAX_CHAR_PER_LINE 512
#define LAST_WORD(X) (X-1)
#define WITHOUT_LAST_WORD(X) (X-1)

typedef struct{
    pid_t pid;
    char** command;
    int commandSize;
    bool background;
}Job;

int getFreePlace( int jobsPlaces[MAX_JOBS]){
    int i = 0;
    while (i < MAX_JOBS){
        if(jobsPlaces[i] == EMPTY){
            return i;
        }
        i++;
    }
    return FAILURE;
}

char* getCommand(){
    char* command;
    char forEnter;
    char buffer [MAX_CHAR_PER_LINE];
    scanf(SCAN_WITHOUT_SPACES,buffer);
    scanf("%c",&forEnter);
    // Allocate the length of the command plus one char for the \0
    command = (char*)malloc(ADD_CHAR(strlen(buffer)));
    if(command==NULL){
        exit(EXIT_FAILURE);
    }
    strcpy(command,buffer);
    return command;
}


void printArgs(char** args, int size){
    printf("size of command = %d\n",size);
    for(int i=0;i<size;i++){
        printf("%s\n",args[i]);
    }
}

char** createArgs(char* command, int* size){
    printf("command = %s\n",command);
    char** args = NULL;
    int i = INITIATE_VALUE;
    char* token = strtok(command,SPACE);
    char* word;
    while (token != NULL){
        args = realloc(args,SIZE_TO_ALLOCATE(i)*sizeof(char*));
        word = (char*)malloc(ADD_CHAR(strlen(token)));
        if (args == NULL || word == NULL){
            exit(EXIT_FAILURE);
        }
        strcpy(word,token);
        token = strtok(NULL,SPACE);
        args[i] = word;
        i++;
    }
    *size = i;
    return args;
}

Job* createJob(char** args,int argsSize){
    Job* job = (Job*)malloc(sizeof(Job));
    // Allocation check
    if (job == NULL){
        exit(EXIT_FAILURE);
    }
    if(strcmp(args[LAST_WORD(argsSize)],BACKGROUND)== IDENTICAL){
        job->background = true;
        job->commandSize = WITHOUT_LAST_WORD(argsSize);
        free(args[LAST_WORD(argsSize)]);
        args = realloc(args, sizeof(char*)*WITHOUT_LAST_WORD(argsSize));
        // Allocation check
        if (args == NULL){
            exit(EXIT_FAILURE);
        }
        job->command = args;
    } else{
        job->background = false;
        job->commandSize = argsSize;
        job->command = args;
    }
}

int main(){
    bool keepRun = true;
    char** args = NULL;
    int argsSize;
    int jobsPlaces[MAX_JOBS]={INITIATE_VALUE};
    Job* jobs[MAX_JOBS];
    Job* job;
    int place;
    while (keepRun){
        printf(PROMPT);
        args = createArgs(getCommand(),&argsSize);
        job = createJob(args,argsSize);
        place = getFreePlace(jobsPlaces);
        jobsPlaces[place] = NOT_EMPTY;
        jobs[place] = job;
    }
    return 0;
}