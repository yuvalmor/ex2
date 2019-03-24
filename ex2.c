//  Yuval Mor - 205380173

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

#define MAX_NUM_JOBS 250
#define MAX_CHAR_PER_LINE 250
#define PROMPT ">"
#define SPACE " "

typedef struct{
    pid_t pid;
    char* command;
}Job;

char** getToken(char* command, int* size){
    char** args = NULL;
    char* token = strtok(command,SPACE);
    int i = 0;
    while (token != NULL){
        args = realloc(args,(i+1)* sizeof(char*));
        if (args == NULL){
            exit(-1);
        }
        args[i] = token;
        token = strtok(NULL,SPACE);
        i++;
    }
    *size = i;
    return args;
}

void freeJob(Job* job){
    free(job->command);
}

void printJob(Job* job){
    printf("%d %s",job->pid,job->command);
}

char* getCommand(){
    char* command;
    char line [MAX_CHAR_PER_LINE];
    scanf("%[^\n]",line);
    command = (char*)malloc(strlen(line)+1);
    strcpy(command,line);
    return command;
}

int main() {
    int size;
    char** args =NULL;
    bool keepRunning = true;
    char* command;
    while (keepRunning){
        printf(PROMPT);
        command = getCommand();
        args = getToken(command,&size);
        keepRunning=false;
    }
    return 0;
}