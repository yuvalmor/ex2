// Yuval Mor - 205380173

#include <stdbool.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define SPACE " "
#define PROMPT ">"
#define END_LINE '\0'
#define BACKGROUND "&"
#define ENTER_BUFFER "%c"
#define CD_COMMAND "cd"
#define EXIT_COMMAND "exit"
#define JOBS_COMMAND "jobs"
#define ERROR_MESSAGE "Error in system call\n"
#define PRINT_PID "%d\n"
#define MAX_JOBS 512
#define MAX_CHAR_PER_LINE 512
#define NOT_EMPTY 1
#define FAILURE -1
#define SYSTEM_CALL_FAILURE -1
#define EMPTY 0
#define INITIATE_VALUE 0
#define IDENTICAL 0
#define FIRST_PLACE 0
#define CHILDREN_PID 0
#define SIZE_TO_ALLOCATE(X) (X+1)
#define ADD_CHAR(X) (X+1)
#define LAST_WORD(X) (X-1)
#define WITHOUT_LAST_WORD(X) (X-1)
#define LAST_CHAR(X) (X-1)

void printArgs(char** args, int size){
    int i=0;
    for(;i < size;i++){
        printf("%s\n",args[i]);
    }
}

typedef struct{
    pid_t pid;
    int place;
    char** command;
    int commandSize;
    bool background;
    bool special;
}Job;

int getFreePlace(const int jobsPlaces[MAX_JOBS]){
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
    char* command = "";
    char buffer [MAX_CHAR_PER_LINE];
    fgets(buffer,MAX_CHAR_PER_LINE,stdin);
    // Remove \n from the line
    buffer[LAST_CHAR(strlen(buffer))] = END_LINE;
    // Allocate the length of the command plus one char for the \0
    if(strlen(buffer)){
        command = (char*)malloc(strlen(buffer));
        if(command == NULL){
            exit(EXIT_FAILURE);
        }
        strcpy(command,buffer);
    }
    return command;
}

char** createArgs(char* command, int* size){
    char** args = (char**)malloc(sizeof(char*));
    int i = INITIATE_VALUE;
    char* token = strtok(command,SPACE);
    char* word;
    while (token != NULL){
       /*if(i>0){
            args = realloc(args,SIZE_TO_ALLOCATE(i)*sizeof(char*));
        }*/
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
    free(command);
    return args;
}

Job* createJob(char** args,int argsSize,int place){
    Job* job = (Job*)malloc(sizeof(Job));
    // Allocation check
    if (job == NULL){
        exit(EXIT_FAILURE);
    }
    job->place = place;
    if(strcmp(args[LAST_WORD(argsSize)],BACKGROUND)== IDENTICAL){
        job->background = true;
        job->commandSize = WITHOUT_LAST_WORD(argsSize);
        free(args[LAST_WORD(argsSize)]);
        args[LAST_WORD(argsSize)] = NULL;
        args = realloc(args, sizeof(char*)*WITHOUT_LAST_WORD(argsSize));
        // Allocation check
        if (args == NULL){
            exit(EXIT_FAILURE);
        }
        job->command = args;
        job->special = false;
    } else{
        if(args[FIRST_PLACE] == CD_COMMAND ||
           args[FIRST_PLACE] == EXIT_COMMAND ||
           args[FIRST_PLACE] == JOBS_COMMAND){
            job->special = true;
            job->background = true;
        } else{
            job->special = false;
            job->background = false;
        }
        job->commandSize = argsSize;
        job->command = args;
    }
}

void deleteJob(Job* job,int jobsPlaces[MAX_JOBS]){
    jobsPlaces[job->place] = EMPTY;
    int i=0;
    for(;i<job->commandSize;i++){
        free(job->command[i]);
        job->command[i] = NULL;
    }
    free(job->command);
    free(job);
}

void preformJob(Job* job){
    pid_t pid;
    if((pid = fork()) == SYSTEM_CALL_FAILURE){
        fprintf(stderr,ERROR_MESSAGE);
    }
    if(pid == CHILDREN_PID) {
        job->pid=getpid();
        printf(PRINT_PID,job->pid);
        execvp(job->command[FIRST_PLACE],job->command);
        fprintf(stderr,ERROR_MESSAGE);
    }
    if(waitpid(pid,NULL,INITIATE_VALUE) == SYSTEM_CALL_FAILURE && errno != ECHILD) {
        fprintf(stderr,ERROR_MESSAGE);
    }
}

int main(){
    int i=0;
    bool keepRun = true;
    char* command;
    char** args = NULL;
    int argsSize;
    int jobsPlaces[MAX_JOBS]={INITIATE_VALUE};
    Job* jobs[MAX_JOBS];
    Job* job;
    int place;
    while (keepRun){
        printf(PROMPT);
        command = getCommand();
        if(strlen(command)==0){
            printf("enter\n");
        } else {
            printf("line\n");
        }
        /*
        args = createArgs(command,&argsSize);
        place = getFreePlace(jobsPlaces);
        jobsPlaces[place] = NOT_EMPTY;
        job = createJob(args,argsSize,place);
        jobs[place] = job;
        if(!job->background){
            preformJob(job);
            deleteJob(job,jobsPlaces);
        } else {
        }
        i++;
        if(i==3){
            keepRun = false;
        }*/
    }
    return 0;
}