/**
 * Shell
 * CS 241 - Spring 2020
 */
#include "format.h"
#include "shell.h"
#include "vector.h"
#include "sstring.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <signal.h> 
#include <sys/types.h>
#include <sys/wait.h>

typedef struct process {
    char *command;
    pid_t pid;
} process;

static vector* history;
static char* history_filename = NULL;
static char* script_filename = NULL;

void input_loop(void);
int handle_input(char*);
int do_cd(char*);
void cleanup_and_exit(void);
void print_history(void);
void print_history_idx(size_t);
void caught_sigint();
void history_match(char*);
int handle_external_command(char*);
void handle_args(int, char*[]);
void load_history();
void load_script();
void save_history();
int check_and_op(char*);
int check_or_op(char*);
int check_semi_op(char*);
int exec_external(char*, char**);

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

int handle_input(char* input){
    int result = 0;
    int len = strlen(input);
    if(len == 0){
        free(input);
        input = NULL;
        return result;
    }
    if(input[len - 1] == '\n'){
        input[len - 1] = '\0';
    }
    //handle first char in input is EOF
    if(input[0] == EOF || input[len - 1] == EOF){
        free(input);
        cleanup_and_exit();
        return result;
    }
    char* copy = (char*)malloc(len + 1);
    strcpy(copy, input);
    char* token = strtok(input, " ");
    if(strcmp(token, "exit") == 0){
        free(input);
        free(copy);
        cleanup_and_exit();
        return result;
    }
    else if(strcmp(token, "!history") == 0){
        print_history();
        free(input);
        free(copy);
        return result;
    } 
    else if(copy[0] == '#'){
        size_t idx;
        sscanf(copy, "#%lu", &idx);
        print_history_idx(idx);
        free(input);
        free(copy);
        return result;
    }
    else if(copy[0] == '!'){
        history_match(copy + 1);
        free(input);
        free(copy);
        return result;
    }

    //all of these commands must be saved in history
    vector_push_back(history, copy);
    if(check_and_op(copy) == 0){
        free(input);
        free(copy);
        return result;
    }
    else if(check_or_op(copy) == 0){
        free(input);
        free(copy);
        return result;
    }
    else if(check_semi_op(copy) == 0){
        free(input);
        free(copy);
        return result;
    }  
    if(strcmp(token, "cd") == 0){
        token = strtok(NULL, " ");
        result = do_cd(token);
    }
    else{
        result = handle_external_command(copy);
    }
    free(input);
    //input = NULL;
    free(copy);
    return result;
}
int handle_external_command(char* input){
    char* input_copy = malloc(strlen(input) + 1);
    strcpy(input_copy, input);
    char* token = strtok(input, " ");
    size_t i = 1;
    char** args = malloc(i * sizeof(char*));
    while(token != NULL){
        char* copy = malloc(strlen(token) + 1);
        strcpy(copy, token);
        args[i - 1] = copy;
        i++;
        args = realloc(args, i * sizeof(char*));
        token = strtok(NULL, " ");
    }
    args[i - 1] = NULL;
    fflush(stdout);
    int res = exec_external(input_copy, args);
    for(size_t j = 0; j < i; j++){
        free(args[j]);
    }
    free(args);
    free(input_copy);
    return res;
}

int exec_external(char* command, char** args){
    pid_t f = fork();
    if(f == -1){
        print_fork_failed();
        return 1;
    }
    //must be child process
    else if(f == 0){
        print_command_executed(getpid());
        execvp(args[0], args);
        print_exec_failed(command);
        print_invalid_command(command);
        exit(1);
    }
    int status;
    int res = waitpid(f, &status, 0);
    if(res == -1){
        print_wait_failed();
    }
    int exit_status = WEXITSTATUS(status);
    if(exit_status != 0){
        return 1;
    }
    else{
        return 0;
    }
}

int do_cd(char* dir){
    if(dir == NULL){
        print_no_directory("");
        return 1;
    }
    int res = chdir(dir);
    if(res == -1){
        print_no_directory(dir);
        return 1;
    }
    return 0;
}

void history_match(char* prefix){
    size_t his_size = vector_size(history);
    size_t prefix_len = strlen(prefix);

    for(size_t i = his_size; i --> 0 ;){
        char* his = (char*)vector_get(history, i);
        if(strncmp(prefix, his, prefix_len) == 0) {
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
        printf("copy: %s\n", copy);
        handle_input(copy);
    }
    return;
}

void caught_sigint(){
    cleanup_and_exit();
}

int check_and_op(char* command){
    sstring* res = cstr_to_sstring(command);
    vector* splits = sstring_split(res, '&');
    if(vector_size(splits) != 3){
       sstring_destroy(res);
       vector_destroy(splits);
       return 1;
    }
    char* command1 = (char*)malloc(strlen(vector_get(splits, 0)) + 1);
    char* command2 = (char*)malloc(strlen(vector_get(splits, 2)) + 1);
    strcpy(command1, vector_get(splits, 0));
    strcpy(command2, vector_get(splits, 2));
    //if first command succeeds, do second
    if(handle_input(command1) == 0){
        handle_input(command2);
        vector_pop_back(history);
    }
    sstring_destroy(res);
    vector_destroy(splits);
    vector_pop_back(history);
    return 0;
}

int check_or_op(char* command){
    sstring* res = cstr_to_sstring(command);
    vector* splits = sstring_split(res, '|');
    if(vector_size(splits) != 3){
       sstring_destroy(res);
       vector_destroy(splits);
       return 1;
    }
    char* command1 = (char*)malloc(strlen(vector_get(splits, 0)) + 1);
    char* command2 = (char*)malloc(strlen(vector_get(splits, 2)) + 1);
    strcpy(command1, vector_get(splits, 0));
    strcpy(command2, vector_get(splits, 2));

    //if first command fails, do second
    if(handle_input(command1) == 1){
        handle_input(command2);
        vector_pop_back(history);
    }
    sstring_destroy(res);
    vector_destroy(splits);
    vector_pop_back(history);
    return 0;
}
int check_semi_op(char* command){
    sstring* res = cstr_to_sstring(command);
    vector* splits = sstring_split(res, ';');
    if(vector_size(splits) != 2){
       sstring_destroy(res);
        vector_destroy(splits);
        return 1;
    }
    char* command1 = (char*)malloc(strlen(vector_get(splits, 0)) + 1);
    char* command2 = (char*)malloc(strlen(vector_get(splits, 1)) + 1);
    strcpy(command1, vector_get(splits, 0));
    strcpy(command2, vector_get(splits, 1));

    //if first command fails, do second
    handle_input(command1);
    handle_input(command2);
    sstring_destroy(res);
    vector_destroy(splits);
    vector_pop_back(history);
    vector_pop_back(history);
    return 0;
}

void cleanup_and_exit(void){
    fflush(stdout);
    save_history();
    vector_destroy(history);
    free(script_filename);
    free(history_filename);
    exit(0);
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
    if(script_filename && history_filename){
        if(argc != 5){
            print_usage();
            cleanup_and_exit();
        }
        return;
    }
    else if(script_filename || history_filename){ 
        if(argc != 3){
            print_usage();
            cleanup_and_exit();
        }
        return;
    }
    else{
        if(argc != 1){
            print_usage();
            cleanup_and_exit();
        }
        return;
    }
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
    if(history_filename == NULL){
        return;
    }
    FILE* f = fopen(history_filename, "r");
    if(f == NULL){
        return;
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

void save_history(){
    if(history_filename == NULL){
        return;
    }
    FILE* f = fopen(history_filename, "w+");
    size_t max_size = vector_size(history);
    for(size_t i = 0; i < max_size; i++){
        char* current = (char*)vector_get(history, i);
        //printf("current: %s\n", current);
        fwrite(current , 1 , strlen(current) , f);
        if(i != (max_size - 1)){
            fwrite("\n" , 1 , 1 , f);
        }
       // fwrite("***", 1, 3, f);
    }
    fclose(f);
    return;
}