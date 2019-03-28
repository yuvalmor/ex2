#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
/*
int main() {
    int value = 3;
    if(fork()!=0){
        wait(&value);
    } else {
        exit(value);
        printf("okk");
    }
    value = WEXITSTATUS(value);
    value++;
    printf("%d",value);
    return 0;
}
*/