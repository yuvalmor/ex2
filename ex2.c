// Yuval Mor - 205380173
/*
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

#define MAX_CHAR_PER_LINE 250
#define MAX_JOBS 512
#define PROMPT ">"
#define SPACE " "
#define BACKGROUND "&"
#define SYSTEM_CALL_FAILURE -1
#define FAILURE -1
#define FIRST_PLACE 0
#define CHILDREN_PID 0
#define IDENTICAL 0
#define EMPTY 0
#define NOT_EMPTY 1
#define FORK_ERROR "fork error"
#define EXECVP_ERROR "execvp error"
#define WAIT_ERROR "wait error"
#define SCAN_WITHOUT_SPACES "%[^\n]"
#define ADD_CHAR(x) (x+1)
#define LAST_PLACE(x) (x-1)

typedef struct{
    pid_t pid;
    char** command;
    int commandSize;
}Job;

char** getToken(char* command, int* size){
    char** args = NULL;
    char* token = strtok(command,SPACE);
    int i = 0;
    while (token != NULL){
        args = realloc(args,(i+1)* sizeof(char*));
        if (args == NULL){
            exit(EXIT_FAILURE);
        }
        args[i] = token;
        token = strtok(NULL,SPACE);
        i++;
    }
    *size = i;
    return args;
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

int findFreePlace(const int jobsPlaces[MAX_JOBS]){
    int i = 0;
    while (i < MAX_JOBS){
        if(jobsPlaces[i] == EMPTY){
            return i;
        }
        i++;
    }
    return FAILURE;
}

Job* newJob(pid_t pid,char** command, int commandSize){
    Job* job = (Job*)malloc(sizeof(Job));
    job->pid=pid;
    job->command = command;
    job->commandSize = commandSize;
    return job;
}

void addJob(Job* jobs[MAX_JOBS],int jobsPlaces[MAX_JOBS],pid_t pid,char** command, int commandSize){
    int place = findFreePlace(jobsPlaces);
    jobsPlaces[place] = NOT_EMPTY;
    jobs[place] = newJob(pid,command,commandSize);
}

void freeJob(Job* jobToFree){
    free(jobToFree->command);
    free(jobToFree);
}

void deleteJob(Job* jobs[MAX_JOBS],int jobsPlaces[MAX_JOBS],pid_t pid){
    bool flag = true;
    int i = 0;
    while (flag){
        if(jobs[i]->pid == pid){
            flag = false;
            freeJob(jobs[i]);
            jobsPlaces[i] = 0;
        }
    }
}

void preformJob(char** args,Job* jobs[MAX_JOBS],int jobsPlaces[MAX_JOBS],int commandSize){
    pid_t pid;
    if((pid = fork()) == SYSTEM_CALL_FAILURE){
        perror(FORK_ERROR);
    }
    if(pid == CHILDREN_PID){
        printf("%d\n",getpid());
        execvp(args[FIRST_PLACE],args);
        perror(EXECVP_ERROR);
    }
    addJob(jobs,jobsPlaces,pid,args,commandSize);
    if(waitpid(pid,NULL,0) == SYSTEM_CALL_FAILURE){
        perror(WAIT_ERROR);
    }
    deleteJob(jobs,jobsPlaces,pid);
}

int main() {
    int jobsPlaces[MAX_JOBS]={0};
    Job* jobs[MAX_JOBS];
    int commandSize;
    char** args;
    char* command;
    bool keepRunning = true;
    while (keepRunning){
        printf(PROMPT);
        command = getCommand();
        args = getToken(command,&commandSize);
        if(strcmp(args[LAST_PLACE(commandSize)],BACKGROUND) == IDENTICAL){

        } else {
           preformJob(args,jobs,jobsPlaces,commandSize);
        }
    }
    free(command);
    return 0;
}*/