/**
 * Shell
 * CS 241 - Spring 2020
 */
#include "format.h"
#include "shell.h"
#include "vector.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

typedef struct process {
    char *command;
    pid_t pid;
} process;

static vector* history;

int input_loop(void);
int handle_input(char*);
int do_cd(char*);
int cleanup_and_exit(void);
int print_history(void);
int print_history_idx(size_t);

int shell(int argc, char *argv[]) {
    // TODO: This is the entry point for your shell.
    history = string_vector_create();
    input_loop();
    return 0;
}

int input_loop(){
    pid_t my_pid = getpid();
    while(1){
        char* cur_dir = get_current_dir_name();
        print_prompt(cur_dir,my_pid);
        char* s = NULL;
        size_t n = 0;
        int read = getline(&s, &n, stdin);
        if(read == -1){
            cleanup_and_exit();
        }
        handle_input(s);
        free(cur_dir);
    }
    return 1;
}

int handle_input(char* input){
    //printf("%s\n", input);
    int len = strlen(input);
    if(len == 0){
        free(input);
        return 0;
    }
    if(input[len - 1] == '\n'){
        input[len - 1] = '\0';
    }
    //handle first char in input is EOF
    if(input[0] == EOF){
        free(input);
        return cleanup_and_exit();
    }
    char* copy = (char*)malloc(len + 1);
    strcpy(copy, input);
    char* token = strtok(input, " ");
    if(strcmp(token, "exit") == 0){
        free(input);
        free(copy);
        return cleanup_and_exit();
    } 
    else if(strcmp(token, "cd") == 0){
        token = strtok(NULL, " ");
        do_cd(token);
        vector_push_back(history, copy);
    }
    else if(strcmp(token, "!history") == 0){
        print_history();
    } 
    else if(copy[0] == '#'){
        size_t idx;
        sscanf(copy, "#%lu", &idx);
        print_history_idx(idx);
    } 
    free(input);
    free(copy);
    return 0;
}

int do_cd(char* s){
    if(s == NULL){
        print_no_directory("");
    }
    int res = chdir(s);
    if(res == -1){
        print_no_directory(s);
    }
    return 0;
}

int print_history(void){
    size_t his_size = vector_size(history);
    for(size_t idx = 0; idx < his_size; idx++){
        print_history_line(idx, vector_get(history, idx));
    }
    return 0;
}

int print_history_idx(size_t idx){
    if(idx >= vector_size(history)){
        print_invalid_index();
    }
    else{
        char* match = vector_get(history, idx);
        char* copy = malloc(strlen(match) + 1);
        strcpy(copy, match);
        handle_input(copy);
    }
    return 0;
}

int cleanup_and_exit(void){
    vector_destroy(history);
    exit(1);
}