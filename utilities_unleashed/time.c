/**
 * Utilities Unleashed
 * CS 241 - Spring 2020
 * looked at code from http://www.cs.tufts.edu/comp/111/examples/Time/clock_gettime.c
 */
#include <time.h>
//#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "format.h"
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    struct timespec start; 
    clock_gettime(CLOCK_MONOTONIC, &start); 
    if(argc <= 1){
        print_time_usage();
        return 1;
    }
    struct timespec finish;
    pid_t f = fork();
    if(f == -1){
        print_fork_failed();
        return 1;
    }
    if(f == 0){
        execvp(argv[1], argv + 1);
        print_exec_failed();
        exit(1);
    }
    else{
        int status;
        waitpid(f, &status, 0);
        if(status != 0){
            return 1;
        }
        clock_gettime(CLOCK_MONOTONIC, &finish); 
        double seconds = (double)finish.tv_sec - (double)start.tv_sec;
        double ns = (double)finish.tv_nsec - (double)start.tv_nsec; 
        double dur = seconds + ns/1000000000;
        display_results(argv, dur);
    }

    return 0;
}
