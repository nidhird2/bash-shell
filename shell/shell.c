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
#include <signal.h> 

typedef struct process {
    char *command;
    pid_t pid;
} process;

static vector* history;

void input_loop(void);
void handle_input(char*);
void do_cd(char*);
void cleanup_and_exit(void);
void print_history(void);
void print_history_idx(size_t);
void caught_sigint();
void history_match(char*);

int shell(int argc, char *argv[]) {
    // TODO: This is the entry point for your shell.
    signal(SIGINT, caught_sigint); 
    history = string_vector_create();
    input_loop();
    return 0;
}

void input_loop(){
    pid_t my_pid = getpid();
    while(1){
        char* cur_dir = get_current_dir_name();
        print_prompt(cur_dir,my_pid);
        free(cur_dir);
        char* s = NULL;
        size_t n = 0;
        int read = getline(&s, &n, stdin);
        if(read == -1){
            free(s);
            cleanup_and_exit();
        }
        char* copy = (char*)malloc(strlen(s) + 1);
        strcpy(copy, s);
        free(s);
        handle_input(copy);
    }
    return;
}

void handle_input(char* input){
    //printf("%s\n", input);
    int len = strlen(input);
    if(len == 0){
        free(input);
        return;
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
    else if(copy[0] == '!'){
        history_match(copy);
    }
    free(input);
    free(copy);
    return;
}

void do_cd(char* s){
    if(s == NULL){
        print_no_directory("");
    }
    int res = chdir(s);
    if(res == -1){
        print_no_directory(s);
    }
    return;
}

void history_match(char* prefix){
    size_t his_size = vector_size(history);

    for(size_t i = his_size; i --> 0 ;){
        char* his = (char*)vector_get(history, i);
        //printf("i: %lu\n", i);
        //printf("his: %s\n", his);
        //printf("prefix: %s\n", prefix + 1);
        if(strstr(his, prefix+1) != NULL) {
            //printf("HERE\n");
            char* match = his;
            char* copy = malloc(strlen(match) + 1);
            strcpy(copy, match);
            handle_input(copy);
            return;
        }
    }
    print_no_history_match();
    return;
}

void print_history(void){
    size_t his_size = vector_size(history);
    for(size_t idx = 0; idx < his_size; idx++){
        print_history_line(idx, (char*)vector_get(history, idx));
    }
    return;
}

void print_history_idx(size_t idx){
    if(idx >= vector_size(history)){
        print_invalid_index();
    }
    else{
        char* match = (char*)vector_get(history, idx);
        char* copy = malloc(strlen(match) + 1);
        strcpy(copy, match);
        handle_input(copy);
    }
    return;
}

void caught_sigint(){
    cleanup_and_exit();
}

void cleanup_and_exit(void){
    vector_destroy(history);
    exit(1);
}