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
#define QUOTATION '"'
#define HYPHEN '-'
#define EMPTY_STRING ""
#define PROMPT "> "
#define END_LINE '\0'
#define BACKGROUND "&"
#define CD_COMMAND "cd"
#define EXIT_COMMAND "exit"
#define JOBS_COMMAND "jobs"
#define ERROR_MESSAGE "Error in system call\n"
#define PRINT_PID "%d\n"
#define PRINT_PID_SPACE "%d "
#define PRINT_STRING "%s "
#define ENTER "\n"
#define TILDA '~'
#define MAX_JOBS 512
#define MAX_CHAR_PER_LINE 512
#define NOT_EMPTY 1
#define PATH 1
#define ONE_ARG 1
#define FAILURE -1
#define SYSTEM_CALL_FAILURE -1
#define EMPTY 0
#define INITIATE_VALUE 0
#define IDENTICAL 0
#define FIRST_PLACE 0
#define CHILD_PID 0
#define TWO_ARG 2
#define ADD_CHAR(X) (X+1)
#define LAST_WORD(X) (X-1)
#define WITHOUT_LAST_WORD(X) (X-1)
#define LAST_CHAR(X) (X-1)

/**
 * The structure Job holds all the relevant information about the job:
 * It keeps the pid of the process that do the job,
 * The command to preform (and its size),its place in the jobs array,
 * And if its foreground or background command.
 * Special commands are: cd, jobs ans exit.
 */
typedef struct{
    pid_t pid;
    char** command;
    int place;
    int commandSize;
    bool background;
    bool foreground;
}Job;

/**
 * The function getCommand gets from the user the command to preform.
 * It save the input from the user in buffer array, removes the enter char,
 * And than allocate the the relevant space on the heap.
 */
char* getCommand(){
    char* command = EMPTY_STRING;
    char buffer [MAX_CHAR_PER_LINE];
    fgets(buffer,MAX_CHAR_PER_LINE,stdin);
    // Remove \n from the line, by change it to \0
    buffer[LAST_CHAR(strlen(buffer))] = END_LINE;
    // Checks that its not an empty command
    if(strlen(buffer)){
        // Allocate the length of the command plus one char for the \0
        command = (char*)malloc(ADD_CHAR(strlen(buffer)));
        // Allocation checks
        if(command == NULL){
            exit(EXIT_FAILURE);
        }
        strcpy(command,buffer);
    }
    return command;
}

/**
 * The function printCommand - print the command word by word.
 */
void printCommand(char** args, int size){
    int i = INITIATE_VALUE;
    for(;i < size;i++){
        printf(PRINT_STRING,args[i]);
    }
}

/**
 * The function printJobs - print all the jobs that runs in the background.
 * It use the function printCommand.
 */
void printJobs(Job* jobs[MAX_JOBS],const int jobsPlaces[MAX_JOBS]){
    int i = INITIATE_VALUE;
    for (;i < MAX_JOBS; i++){
        if(jobsPlaces[i] == NOT_EMPTY){
            printf(PRINT_PID_SPACE,jobs[i]->pid);
            printCommand(jobs[i]->command,jobs[i]->commandSize);
            printf(ENTER);
        }
    }
}

/**
 * The function getFreePlace return the closest free space in the array.
 * If there is no free space it returns -1.
 */
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

/**
 * The function createArgs get the command,
 * Split it into a dynamic allocate array,
 * And update its size (number of words).
 */
char** createArgs(char* command, int* size){
    char** args = (char**)malloc(MAX_JOBS*sizeof(char*));
    // Allocation check
    if (args == NULL){
        exit(EXIT_FAILURE);
    }
    // Initial the array to NULL
    int j= INITIATE_VALUE;
    for(;j<MAX_JOBS;j++){
        args[j] =NULL;
    }
    int i = INITIATE_VALUE;
    // Split the line into words
    char* token = strtok(command,SPACE);
    char* word;
    while (token != NULL) {
        word = (char*) malloc(ADD_CHAR(strlen(token)));
        // Allocation check
        if (word == NULL) {
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

/**
 * The function createJob initial the job with the relevant information.
 */
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
        job->command = args;
        job->foreground = false;
    } else{
        // Checks if its foreground command - job/exit/cd
        if((strcmp(args[FIRST_PLACE],CD_COMMAND)== IDENTICAL) ||
           (strcmp(args[FIRST_PLACE],EXIT_COMMAND) == IDENTICAL) ||
           (strcmp(args[FIRST_PLACE],JOBS_COMMAND)== IDENTICAL)){
            job->foreground = true;
            job->background = true;
        } else{
            job->foreground = false;
            job->background = false;
        }
        job->commandSize = argsSize;
        job->command = args;
    }
    return job;
}

/**
 * The function deleteJob remove the job from the jobsPlaces,
 * And free the job resources.
 */
void deleteJob(Job* job,int jobsPlaces[MAX_JOBS]){
    jobsPlaces[job->place] = EMPTY;
    int i = INITIATE_VALUE;
    for(;i<job->commandSize;i++){
        // free every word in the command
        free(job->command[i]);
        job->command[i] = NULL;
    }
    // free the command
    free(job->command);
    free(job);
}

/**
 * The function updateJobs - checks if the process in the background
 * Finished their running and if so they will be deleted
 */
void updateJobs(int jobsPlaces[MAX_JOBS],Job* jobs[MAX_JOBS]){
    int i = INITIATE_VALUE;
    int status;
    for(;i<MAX_JOBS;i++){
        if(jobsPlaces[i]== NOT_EMPTY){
            // Checks if the process finished
            if(waitpid(jobs[i]->pid,&status,WNOHANG)){
                deleteJob(jobs[i],jobsPlaces);
            }
        }
    }
}

/**
 * The function cdCommand - responsible for the variation change directory commands.
 */
void cdCommand(Job* job, char* lastDirectory){
    // Print the "father" pid to the screen
    printf(PRINT_PID,getpid());
    char* tempDirectory = NULL;
    tempDirectory = getcwd(tempDirectory,MAX_CHAR_PER_LINE);
    // Check failure of system call getcwd
    if(tempDirectory == NULL){
        fprintf(stderr,ERROR_MESSAGE);
        return;
    }
    // Cases: cd and cd ~
    if(job->commandSize == ONE_ARG || job->command[PATH][FIRST_PLACE] == TILDA){
        char* path = NULL;
        // Save the program default home path into variable path
        path = getenv("HOME");
        if(path == NULL){
            fprintf(stderr,ERROR_MESSAGE);
            return;
        }
        if(job->commandSize==TWO_ARG){
            free(job->command[PATH]);
        }
        job->command[PATH]=path;
        // To prevent extra free to the address
        job->commandSize=ONE_ARG;
    }
    // Case: cd-
    if(job->command[PATH][FIRST_PLACE] == HYPHEN){
        free(job->command[PATH]);
        job->command[PATH] = lastDirectory;
    }
    // Cases: cd "abc" and cd "yuval mor"
    if(job->command[PATH][FIRST_PLACE]== QUOTATION){
        char* fixPath = (char*)malloc(MAX_CHAR_PER_LINE);
        if(fixPath == NULL){
            exit(EXIT_FAILURE);
        }
        int i= PATH;
        int k = INITIATE_VALUE;
        int j;
        // Remove the quotation "" from the command
        for(;i<job->commandSize;i++){
            // Adding space between the words
            if (i!=PATH){
                strcpy(fixPath+k,SPACE);
                k++;
            }
            j=INITIATE_VALUE;
            for (;j<strlen(job->command[i]);j++){
                if(job->command[i][j]!=QUOTATION){
                    fixPath[k] = job->command[i][j];
                    k++;
                }
            }
            free(job->command[i]);
            job->command[i]=NULL;
        }
        job->command[PATH] = fixPath;
        job->commandSize = TWO_ARG;
    }
    // Change the directory
    if(chdir(job->command[PATH]) == SYSTEM_CALL_FAILURE) {
        fprintf(stderr,ERROR_MESSAGE);
    } else {
        // Update the last directory
        strcpy(lastDirectory,tempDirectory);
        free(tempDirectory);
    }
}

/**
 * The function preformSpecialJob preform the foreground special commands as:
 * jobs, cd and exit.
 */
void preformSpecialJob(Job* job,bool* keepRun,int jobsPlaces[MAX_JOBS],
                       Job* jobs[MAX_JOBS],char* lastDirectory){
    // Exit command - change the variable keepRun, that stop the while loop in main
    if(strcmp(job->command[FIRST_PLACE],EXIT_COMMAND) == IDENTICAL){
        printf(PRINT_PID,getpid());
        *keepRun = false;
    }
    // Jobs command - print to the screen all the background running commands
    if(strcmp(job->command[FIRST_PLACE],JOBS_COMMAND) == IDENTICAL){
        deleteJob(job,jobsPlaces);
        updateJobs(jobsPlaces,jobs);
        printJobs(jobs,jobsPlaces);
        return;
    }
    /**
     * change directory job
     */
    if(strcmp(job->command[FIRST_PLACE],CD_COMMAND) == IDENTICAL) {
        cdCommand(job,lastDirectory);
        deleteJob(job,jobsPlaces);
    }
}

/**
 * Preform the job, if its not a background job the father process
 * Wait until the child finish.
 */
void preformJob(Job* job,int jobsPlaces[MAX_JOBS]){
    pid_t pid;
    //Create new process
    if((pid = fork()) == SYSTEM_CALL_FAILURE){
        fprintf(stderr,ERROR_MESSAGE);
    }
    if(pid == CHILD_PID){
        execvp(job->command[FIRST_PLACE],job->command);
        fprintf(stderr, ERROR_MESSAGE);
    } else {
        printf(PRINT_PID,pid);
        job->pid=pid;
        // Wait for the child process to finish
        if(!job->background){
            if(waitpid(pid,NULL,INITIATE_VALUE)==SYSTEM_CALL_FAILURE &&
               errno!=ECHILD) {
                fprintf(stderr,ERROR_MESSAGE);
            }
            // remove the job from the list
            deleteJob(job,jobsPlaces);
        }
    }
}

/**
 * freeAllJobs - free the resources that remain open,
 * Before closing the program.
 */
void freeAllJobs(int jobsPlaces[MAX_JOBS],Job* jobs[MAX_JOBS]){
    int i = INITIATE_VALUE;
    for (;i < MAX_JOBS;i++) {
        if(jobsPlaces[i] == NOT_EMPTY){
            deleteJob(jobs[i],jobsPlaces);
        }
    }
}


int main(){
    // variable that holds the last directory of the process
    char* lastDirectory = NULL;
    lastDirectory = getcwd(lastDirectory,MAX_CHAR_PER_LINE);
    // Check failure on system call getcwd
    if(lastDirectory == NULL){
        fprintf(stderr,ERROR_MESSAGE);
    }
    int jobsPlaces[MAX_JOBS]={INITIATE_VALUE};
    Job* jobs[MAX_JOBS];
    bool keepRun = true;
    char** args = NULL;
    char* command;
    // Number of arguments in the command
    int argsSize;
    int place;
    Job* job;
    while(keepRun){
        printf(PROMPT);
        // Gets the command from the user
        command = getCommand();
        // If its an empty command (only enter) continue to the next iteration
        if(!strlen(command)){
            continue;
        }
        // Split te command into tokens
        args = createArgs(command,&argsSize);
        // Get free place in the array to put the job inside
        place = getFreePlace(jobsPlaces);
        // Update that the place aren't empty
        jobsPlaces[place] = NOT_EMPTY;
        // Create the job
        job = createJob(args,argsSize,place);
        // Place the job in the free space
        jobs[place] = job;
        if(job->foreground){
            preformSpecialJob(job,&keepRun,jobsPlaces,jobs,lastDirectory);
        } else {
            preformJob(job,jobsPlaces);
        }
    }
    // Free all the open resources before ending the program
    freeAllJobs(jobsPlaces,jobs);
    return 0;
}