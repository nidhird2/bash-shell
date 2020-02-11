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
#include <stdbool.h>

typedef struct process {
    char *command;
    pid_t pid;
} process;

static vector* history;
static char* history_filename = NULL;
static char* script_filename = NULL;

void input_loop(void);
void handle_input(char*);
int do_cd(char*);
void cleanup_and_exit(void);
void print_history(void);
void print_history_idx(size_t);
void caught_sigint();
void history_match(char*);
void handle_external_command(char*);
void handle_args(int, char*[]);
void load_history();
void load_script();

int shell(int argc, char *argv[]) {
    // TODO: This is the entry point for your shell.
    signal(SIGINT, caught_sigint); 
    history = string_vector_create();
    handle_args(argc, argv);
    load_history();
    if(script_filename){
        load_script();
    } else{
        input_loop();
    }
    return 0;
}

void handle_args(int argc, char*argv[]){
    opterr = 0;
    char* options = "f:h:";
    int opt = getopt(argc, argv, options);
    while(opt != -1){
        if(opt == 'f'){
            script_filename = get_full_path(optarg);

        }
        else if(opt == 'h'){
            history_filename = get_full_path(optarg);
        }
        else{
            print_usage();
            cleanup_and_exit();
        }
        opt = getopt(argc, argv, options);
    }
    return;
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
    //printf("\n.%s.\n", input);
    int len = strlen(input);
    if(len == 0){
        free(input);
        input = NULL;
        return;
    }
    if(input[len - 1] == '\n'){
        input[len - 1] = '\0';
    }
    //handle first char in input is EOF
    if(input[0] == EOF || input[len - 1] == EOF){
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
    else{
        handle_external_command(copy);
    }
    free(input);
    input = NULL;
    free(copy);
    return;
}
void handle_external_command(char* input){
    //fflush(STDOUT_FILENO);
    return;
}

int do_cd(char* s){
    if(s == NULL){
        print_no_directory("");
        return 1;
    }
    int res = chdir(s);
    if(res == -1){
        print_no_directory(s);
        return 1;
    }
    return 0;
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
    free(script_filename);
    free(history_filename);
    exit(0);
}

void load_script(){
    if(script_filename == NULL){
        return;
    }
    FILE* f = fopen(script_filename, "r");
    if(f == NULL){
        print_script_file_error();
        cleanup_and_exit();
    }
    char * line = NULL;
    size_t len = 0;
    int read = getline(&line, &len, f);

    while (read != -1) {
        if(line[read - 1] == '\n'){
            line[read - 1] = '\0';
        }
        handle_input(line);
        len = 0;
        read = getline(&line, &len, f);
    }
    free(line);
    fclose(f);
    cleanup_and_exit();
}

void load_history(){
    printf("HERE\n");
    if(history_filename == NULL){
        printf("YIKES\n");
        return;
    }
    FILE* f = fopen(history_filename, "r");
    if(f == NULL){
        print_history_file_error();
        cleanup_and_exit();
    }
    char * line = NULL;
    size_t len = 0;
    int read = getline(&line, &len, f);

    while (read != -1) {
        if(line[read - 1] == '\n'){
            line[read - 1] = '\0';
        }
        vector_push_back(history, line);
        read = getline(&line, &len, f);
    }
    free(line);
    fclose(f);
}