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
#define EMPTY_STRING ""
#define PROMPT ">"
#define END_LINE '\0'
#define BACKGROUND "&"
#define CD_COMMAND "cd"
#define EXIT_COMMAND "exit"
#define JOBS_COMMAND "jobs"
#define ERROR_MESSAGE "Error in system call\n"
#define PRINT_PID "%d\n"
#define PRINT_PID_SPACE "%d "
#define ENTER "\n"
#define MAX_JOBS 512
#define MAX_CHAR_PER_LINE 512
#define NOT_EMPTY 1
#define PATH 1
#define FAILURE -1
#define SYSTEM_CALL_FAILURE -1
#define EMPTY 0
#define INITIATE_VALUE 0
#define IDENTICAL 0
#define FIRST_PLACE 0
#define CHILD_PID 0
#define ADD_CHAR(X) (X+1)
#define LAST_WORD(X) (X-1)
#define WITHOUT_LAST_WORD(X) (X-1)
#define LAST_CHAR(X) (X-1)

typedef struct{
    pid_t pid;
    char** command;
    int place;
    int commandSize;
    bool background;
    bool special;
}Job;

void printArgs(char** args, int size){
    int i = INITIATE_VALUE;
    for(;i < size;i++){
        printf("%s ",args[i]);
    }
}

void printJobs(Job* jobs[MAX_JOBS],const int jobsPlaces[MAX_JOBS]){
    int i = INITIATE_VALUE;
    for (;i<MAX_JOBS;i++){
        if(jobsPlaces[i]==NOT_EMPTY){
            printf(PRINT_PID_SPACE,jobs[i]->pid);
            printArgs(jobs[i]->command,jobs[i]->commandSize);
            printf(ENTER);
        }
    }
}

int getFreePlace(const int jobsPlaces[MAX_JOBS]){
    int i = INITIATE_VALUE;
    while (i < MAX_JOBS){
        if(jobsPlaces[i] == EMPTY){
            return i;
        }
        i++;
    }
    return FAILURE;
}

char* getCommand(){
    char* command = EMPTY_STRING;
    char buffer [MAX_CHAR_PER_LINE];
    fgets(buffer,MAX_CHAR_PER_LINE,stdin);
    // Remove \n from the line
    buffer[LAST_CHAR(strlen(buffer))] = END_LINE;
    // Allocate the length of the command plus one char for the \0
    if(strlen(buffer)){
        // ADD ONE - ICHANGE KNOW!!!!!
        command = (char*)malloc(ADD_CHAR(strlen(buffer)));
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
       if(i>0){
            args = realloc(args,ADD_CHAR(i)*sizeof(char*));
        }
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
        args = realloc(args,sizeof(char*)*WITHOUT_LAST_WORD(argsSize));
        // Allocation check
        if (args == NULL){
            exit(EXIT_FAILURE);
        }
        job->command = args;
        job->special = false;
    } else{
        if((strcmp(args[FIRST_PLACE],CD_COMMAND)== IDENTICAL) ||
        (strcmp(args[FIRST_PLACE],EXIT_COMMAND) == IDENTICAL) ||
        (strcmp(args[FIRST_PLACE],JOBS_COMMAND)== IDENTICAL)){
            job->special = true;
            job->background = true;
        } else{
            job->special = false;
            job->background = false;
        }
        job->commandSize = argsSize;
        job->command = args;
    }
    return job;
}

void deleteJob(Job* job,int jobsPlaces[MAX_JOBS]){
    jobsPlaces[job->place] = EMPTY;
    int i = INITIATE_VALUE;
    for(;i<job->commandSize;i++){
        free(job->command[i]);
        job->command[i] = NULL;
    }
    free(job->command);
    free(job);
}

void preformSpecialJob(Job* job,bool* keepRun,
        int jobsPlaces[MAX_JOBS],Job* jobs[MAX_JOBS]){
    if(strcmp(job->command[FIRST_PLACE],EXIT_COMMAND) == IDENTICAL){
        printf(PRINT_PID,getpid());
        *keepRun = false;
        return;
    }
    if(strcmp(job->command[FIRST_PLACE],CD_COMMAND) == IDENTICAL) {
        printf(PRINT_PID,getpid());
        if(chdir(job->command[PATH]) == SYSTEM_CALL_FAILURE){
            fprintf(stderr,ERROR_MESSAGE);
        }
        return;
    }
    if(strcmp(job->command[FIRST_PLACE],JOBS_COMMAND) == IDENTICAL){
        jobsPlaces[job->place] = EMPTY;
        printJobs(jobs,jobsPlaces);
        deleteJob(job,jobsPlaces);
    }
}

void preformJob(Job* job,int jobsPlaces[MAX_JOBS]){
    pid_t pid;
    if((pid = fork()) == SYSTEM_CALL_FAILURE){
        fprintf(stderr,ERROR_MESSAGE);
    }
    if(pid == CHILD_PID){
        execvp(job->command[FIRST_PLACE],job->command);
        fprintf(stderr, ERROR_MESSAGE);
    } else {
        printf(PRINT_PID,pid);
        job->pid=pid;
        if(!job->background){
            if(waitpid(pid,NULL,INITIATE_VALUE)==SYSTEM_CALL_FAILURE &&
            errno!=ECHILD) {
                fprintf(stderr,ERROR_MESSAGE);
            }
            deleteJob(job,jobsPlaces);
        }
    }
}

void updateJobs(int jobsPlaces[MAX_JOBS],Job* jobs[MAX_JOBS]){
    int i = INITIATE_VALUE;
    int status;
    for(;i<MAX_JOBS;i++){
        if(jobsPlaces[i]== NOT_EMPTY){
            if(waitpid(jobs[i]->pid,&status,WNOHANG)){
                jobsPlaces[i] = EMPTY;
            }
        }
    }
}

int main(){
    bool keepRun = true;
    char* command;
    char** args = NULL;
    int argsSize;
    int jobsPlaces[MAX_JOBS]={INITIATE_VALUE};
    Job* jobs[MAX_JOBS];
    Job* job;
    int place;
    while(keepRun){
        updateJobs(jobsPlaces,jobs);
        printf(PROMPT);
        command = getCommand();
        if(strlen(command)) {
            args = createArgs(command,&argsSize);
            place = getFreePlace(jobsPlaces);
            jobsPlaces[place] = NOT_EMPTY;
            job = createJob(args,argsSize,place);
            jobs[place] = job;
            if(job->special){
                updateJobs(jobsPlaces,jobs);
                preformSpecialJob(job,&keepRun,jobsPlaces,jobs);
            } else {
                preformJob(job,jobsPlaces);
            }
        }
    }
    return 0;
}