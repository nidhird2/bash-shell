/**
 * mapreduce
 * CS 241 - Spring 2020
 */
#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void free_indexes(char** indexes, size_t map_count){
    for(size_t i = 0; i < map_count; i++){
        free(indexes[i]);
    }
    free(indexes);
    indexes = NULL;
}

char** get_indexes(size_t map_count){
    char** indexes = (char**)malloc(sizeof(char*) * map_count);
    for(size_t i = 0; i < map_count; i++){
        const int n = snprintf(NULL, 0, "%lu", i);
        char* buf = malloc(n+1);
        snprintf(buf, n+1, "%lu", i);
        indexes[i] = buf;
    }
    return indexes;
}

int main(int argc, char **argv) {
    if(argc != 6){
        print_usage();
        return -1;
    }
    char* input_file_name = argv[1];
    char* output_file_name = argv[2];
    char* mapper_name = argv[3];
    char* reducer_name = argv[4];
    size_t map_count;
    sscanf(argv[5], "%lu", &map_count);
    if(map_count <= 0){
        print_usage();
        return -1;
    }
    // printf("input file name: %s\n", input_file_name);
    // printf("output file name: %s\n", output_file_name);
    // printf("mapper file name: %s\n", mapper_name);
    // printf("reducer file name: %s\n", reducer_name);

    // Create an input pipe for each mapper.
    int map_fds[map_count][2];
    for(size_t i = 0; i < map_count; i++){
        pipe(map_fds[i]);
    }

    // Create one input pipe for the reducer.
    int reducer_fd[2];
    pipe(reducer_fd);

    // Open the output file.

    // Start a splitter process for each mapper.m
    char** indexes = get_indexes(map_count);
    char* splitter_name = "./splitter";
    pid_t split_pids[map_count];
    for(size_t i = 0; i < map_count; i++){
        pid_t pid = fork();
        if(pid == 0){
            close(map_fds[i][0]);
            dup2(map_fds[i][1], STDOUT_FILENO);
            execlp(splitter_name, splitter_name, input_file_name, argv[5], indexes[i], (char*)NULL);
            return -2;
        }
        else{
            close(map_fds[i][1]);
            split_pids[i] = pid;
        }
    }
    // Start all the mapper processes.
    pid_t map_pids[map_count];
    for(size_t i = 0; i < map_count; i++){
        pid_t pid = fork();
        if(pid == 0){
            //read from pipe
            dup2(map_fds[i][0], STDIN_FILENO);
            //output to reducer
            dup2(reducer_fd[1], STDOUT_FILENO);
            execlp(mapper_name, mapper_name, (char*)NULL);
            return -3;
        }
        else{
            close(map_fds[i][0]);
            map_pids[i] = pid;
        }
    }
    close(reducer_fd[1]);

    // Start the reducer process.
    pid_t pid = fork();
    if(pid == 0){
        //read from input pipe
        FILE* output_file = fopen(output_file_name, "w");
        dup2(reducer_fd[0], STDIN_FILENO);
        // output to output_file
        dup2(fileno(output_file), STDOUT_FILENO);
        execlp(reducer_name, reducer_name, (char*)NULL);
        return -4;
    }
    close(reducer_fd[0]);

    // Wait for the reducer to finish.
    int status;
    waitpid(pid, &status, 0);
    close(reducer_fd[0]);

    // Print nonzero subprocess exit codes.
    //check split status codes
    for(size_t i  = 0; i < map_count; i++){
        int status2;
        waitpid(split_pids[i], &status2, 0);
        if(WIFEXITED(status2) && WEXITSTATUS(status2) ==0){
            continue;
        }
        char example[300];
        strcpy(example, "./splitter");
        strcat(example, input_file_name);
        strcat(example, argv[5]);
        strcat(example, indexes[i]);
        print_nonzero_exit_status(reducer_name, WEXITSTATUS(status2));
    }
    //check map status codes
    for(size_t i  = 0; i < map_count; i++){
        int status2;
        waitpid(map_pids[i], &status2, 0);
        if(WIFEXITED(status2) && WEXITSTATUS(status2) != 0){
            print_nonzero_exit_status(mapper_name, WEXITSTATUS(status2));
        }
    }
    if(WIFEXITED(status) && WEXITSTATUS(status) !=0){
        print_nonzero_exit_status(reducer_name, WEXITSTATUS(status));
    }

    // Count the number of lines in the output file.
    print_num_lines(output_file_name);
    free(indexes);
    return 0;
}
