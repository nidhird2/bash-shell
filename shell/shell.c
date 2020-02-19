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
#include <time.h>

typedef struct process {
    char *command;
    pid_t pid;
} process;

static vector* history;
static char* history_filename = NULL;
static char* script_filename = NULL;
static pid_t foreground;
static vector* processes;

char* calc_time_str(unsigned long, unsigned long);
char* calc_start_time(unsigned long long);
process_info* get_info(int);
void ps();
void input_loop(void);
void reap_children();
int handle_input(char*);
int do_cd(char*);
void cleanup(void);
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
int send_kill(char*, int);
int send_stop(char*, int);
int send_cont(char*, int);
process* find_process_pid(int);
char** getargs(char* input);
int exec_background(char*, char**);
int handle_background_command(char*);
process* create_process(char*, pid_t);
void destroy_process(process*);
void read_children();
void kill_and_clean_children();
int redirect_output(char*);

int redirect_append(char*);

int redirect_input(char*);


int shell(int argc, char *argv[]) {
    // TODO: This is the entry point for your shell.
    signal(SIGINT, caught_sigint); 
    history = string_vector_create();
    processes = vector_create(NULL, NULL, NULL);
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
            cleanup();
            exit(0);
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
        cleanup();
        exit(0);
        return result;
    }
    char* copy = (char*)malloc(len + 1);
    strcpy(copy, input);
    char* token = strtok(input, " ");
    if(strcmp(token, "exit") == 0){
        free(input);
        free(copy);
        cleanup();
        exit(0);
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
    if(strcmp(token, "ps") == 0){
        ps();
        free(input);
        free(copy);
        return result;
    }
    else if(check_and_op(copy) == 0){
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
    } else if(strstr(copy, " > ") != NULL){
        result = redirect_output(copy);
    } else if(strstr(copy, " >> ") != NULL){
        result = redirect_append(copy);
    } else if(strstr(copy, " < ") != NULL){
        result = redirect_input(copy);
    }
    else if(strrchr(copy, '&') != NULL){
        char* loc = strrchr(copy, '&');
        *loc = '\0';
        result = handle_background_command(copy);
    }  
    else if(strcmp(token, "cd") == 0){
        token = strtok(NULL, " ");
        result = do_cd(token);
    }
    else if(strcmp(token, "kill") == 0){
        token = strtok(NULL, " ");
        int pid = 0;
        if(token){
            sscanf(token, "%d", &pid);
        }
        result = send_kill(copy, pid);
    }
    else if(strcmp(token, "stop") == 0){
        token = strtok(NULL, " ");
        int pid = 0;
        if(token){
            sscanf(token, "%d", &pid);
        }
        result = send_stop(copy, pid);
    }
    else if(strcmp(token, "cont") == 0){
        token = strtok(NULL, " ");
        int pid = 0;
        if(token){
            sscanf(token, "%d", &pid);
        }
        result = send_cont(copy, pid);
    }
    else{
        result = handle_external_command(copy);
    }
    free(input);
    input = NULL;
    free(copy);
    return result;
}

int handle_external_command(char* input){
    char* input_copy = malloc(strlen(input) + 1);
    strcpy(input_copy, input);
    char** args = getargs(input);
    fflush(stdout);
    fflush(stdin);
    int res = exec_external(input_copy, args);
    free(input_copy);
    return res;
}

int handle_background_command(char* input){
    char* input_copy = malloc(strlen(input) + 1);
    strcpy(input_copy, input);
    char** args = getargs(input);
    fflush(stdout);
    fflush(stdin);
    int res = exec_background(input_copy, args);
    free(input_copy);
    return res;
}

char** getargs(char* input){
    char* token = strtok(input, " ");
    size_t i = 1;
    char** args = malloc(sizeof(char*));
    while(token != NULL){
        char* copy = malloc(strlen(token) + 1);
        strcpy(copy, token);
        args[i - 1] = copy;
        i++;
        args = realloc(args, i * sizeof(char*));
        token = strtok(NULL, " ");
    }
    args[i - 1] = NULL;
    return args;
}

int exec_background(char* command, char** args){
    reap_children();
    fflush(stdout);
    fflush(stdin);
    pid_t f = fork();
    if(f == -1){
        print_fork_failed();
        return 1;
    } else if (f == 0){
        fflush(stdout);
        fflush(stdin);
        print_command_executed(getpid());
        execvp(args[0], args);
        print_exec_failed(command);
        print_invalid_command(command);
        size_t i = 0;
        while(args[i] != NULL){
            free(args[i]);
            i++;
        }
        free(args);
        cleanup();
        exit(1);
    } else {
        if(setpgid(f, f) != 0){
            print_setpgid_failed();
        }
        process* p1 = create_process(command, f);
        vector_push_back(processes, p1);
        size_t i = 0;
        while(args[i] != NULL){
            free(args[i]);
            i++;
        }
        free(args);
        return 1;
    }
}

int exec_external(char* command, char** args){
    reap_children();
    fflush(stdout);
    fflush(stdin);
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
        size_t i = 0;
        while(args[i] != NULL){
            free(args[i]);
            i++;
        }
        free(args);
        cleanup();
        exit(1);
    }
    if(setpgid(f, f) != 0){
            print_setpgid_failed();
    }
    foreground = f;
    size_t i = 0;
    while(args[i] != NULL){
        free(args[i]);
        i++;
    }
    free(args);
    int status;
    int res = waitpid(f, &status, 0);
    fflush(stdout);
    fflush(stdin);
    if(res == -1){
        print_wait_failed();
    }
    foreground = 0;
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
        handle_input(copy);
    }
    return;
}

void caught_sigint(){
    //printf("pid_t foreground: %d\n", foreground);
    if(foreground != 0){
        kill(foreground, SIGKILL);
    }
    //input_loop();
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

void cleanup(void){
    fflush(stdout);
    save_history();
    vector_destroy(history);
    kill_and_clean_children();
    vector_destroy(processes);
    free(script_filename);
    free(history_filename);
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
            cleanup();
            exit(0);
        }
        opt = getopt(argc, argv, options);
    }
    if(script_filename && history_filename){
        if(argc != 5){
            print_usage();
            cleanup();
            exit(0);
        }
        return;
    }
    else if(script_filename || history_filename){ 
        if(argc != 3){
            print_usage();
            cleanup();
            exit(0);
        }
        return;
    }
    else{
        if(argc != 1){
            print_usage();
            cleanup();
            exit(0);
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
        cleanup();
        exit(0);
    }
    char * line = NULL;
    size_t len = 0;
    int read = getline(&line, &len, f);

    pid_t my_pid = getpid();

    while (read != -1) {
        char* cur_dir = get_current_dir_name();
        print_prompt(cur_dir,my_pid);
        printf("%s", line);
        if(line[read - 1] == '\n'){
            line[read - 1] = '\0';
        }
        handle_input(line);
        len = 0;
        read = getline(&line, &len, f);
        free(cur_dir);
    }
    free(line);
    fclose(f);
    cleanup();
    exit(0);
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

int send_kill(char* command, int pid){
    if(pid == 0){
        print_invalid_command(command);
        return 1;
    }
    reap_children();
    process* found = find_process_pid(pid);
    if(found == NULL){
        print_no_process_found(pid);
        return 1;
    }
    kill(pid, SIGTERM);
    print_killed_process(pid, found->command);
    reap_children();
    return 0; 
}

int send_stop(char* command, pid_t pid){
    if(pid == 0){
        print_invalid_command(command);
        return 1;
    }
    reap_children();
    process* found = find_process_pid(pid);
    if(found == NULL){
        print_no_process_found(pid);
        return 1;
    }

    kill(pid, SIGTSTP);
    printf("stop sent\n");
    print_stopped_process(pid, found->command);
    reap_children();
    return 0;
}

int send_cont(char* command, pid_t pid){
    if(pid == 0){
        print_invalid_command(command);
        return 1;
    }
    reap_children();
    process* found = find_process_pid(pid);
    if(found == NULL){
        print_no_process_found(pid);
        return 1;
    }
    kill(pid, SIGCONT);
    print_continued_process(pid, found->command);
    return 1;
}

process* find_process_pid(int input_pid){
    size_t pro_size = vector_size(processes);
    for(size_t i = 0; i < pro_size; i++){
        process* current = (process*)vector_get(processes, i);
        if(current ->pid == input_pid){
            return current;
        }
    }
    return NULL;
}

process* create_process(char* command, pid_t id){
    process* result = (process*)malloc(sizeof(process));
    result->pid = id;
    char* cpy = (char*)malloc(strlen(command) + 1);
    strcpy(cpy, command);
    result->command = cpy;
    return result;
}

void destroy_process(process* input){
    free(input->command);
    free(input);
    return;
}

void reap_children(){
    size_t i = 0;
    while(i < vector_size(processes)){
        process* current = (process*)vector_get(processes, i);
        pid_t pid = current->pid;
        int status;
        if(0 < waitpid(pid, &status, WNOHANG)){
            destroy_process(current);
            current = NULL;
            vector_erase(processes, i);
        } else{
            i++;
        }
    }
}

void kill_and_clean_children(){
    size_t max = vector_size(processes);
    for(size_t i = 0; i < max; i++){
        process* current = (process*)vector_get(processes, i);
        kill(current->pid, SIGKILL);
        int status;
        waitpid(current->pid, &status, 0);
        destroy_process(current);
        current = NULL;
    }
    return;
}

void ps(){
    print_process_info_header();
    reap_children();
    size_t max = vector_size(processes);
    process_info* info = get_info((int)getpid());
    info->command = "./shell";
    print_process_info(info);
    free(info->start_str);
    free(info->time_str);
    free(info);

    for(size_t i = 0; i < max; i++){
        process* current = (process*)vector_get(processes, i);
        process_info* info = get_info((int)current->pid);
        info->command = current->command;
        print_process_info(info);
        free(info->start_str);
        free(info->time_str);
        free(info);
    }
}

process_info* get_info(int pid){
    process_info* info = (process_info*)malloc(sizeof(process_info));
    info->pid = pid;
    info->nthreads = 0;
    info->vsize = 0;
    info->state = ' ';
    info->start_str = "";
    info->time_str = "";
    char path[40];
    char* line = NULL;
    size_t cap = 0;
    snprintf(path, 40, "/proc/%d/stat", pid);
    FILE* f = fopen(path, "r");
    getline(&line, &cap, f);
    fclose(f);
    char* token = strtok(line, " ");
    int count = 1;
    unsigned long utime;
    unsigned long stime;
    unsigned long long starttime;

    while(token != NULL){
        //must be state
        if(count == 3){
            char state;
            sscanf(token, "%c", &state);
            info->state = state;
        //must be utime
        } else if(count == 14){
            sscanf(token, "%lu", &utime);
        //must be stime
        } else if(count == 15){
            sscanf(token, "%lu", &stime);
        //must be num_threads
        } else if(count == 20){
            long int threads;
            sscanf(token, "%ld", &threads);
            info->nthreads = threads;
        //must be starttime
        } else if(count == 22){
            sscanf(token, "%llu", &starttime);
        //must be vsize
        } else if(count == 23){
            unsigned long bytes;
            sscanf(token, "%lu", &bytes);
            //CONVERT TO KB;
            unsigned long int kb = bytes / 1024;
            info->vsize = kb;
        } 
        token = strtok(NULL, " ");
        count++;
    }
    free(line);
    info->start_str = calc_start_time(starttime);
    info->time_str = calc_time_str(utime, stime);
    return info;
}

char* calc_start_time(unsigned long long starttime){
    starttime = starttime / sysconf(_SC_CLK_TCK);
    unsigned long long boot_time = 0;
    FILE* f = fopen("/proc/stat", "r");
    char* line = NULL;
    size_t cap = 0;
    while(getline(&line, &cap, f) != -1){
        if(strncmp(line, "btime", 5) != 0){
            continue;
        }
        sscanf(line + 7, "%llu", &boot_time);
        break;
    }
    free(line);
    fclose(f);
    char* buf = malloc(20);
    unsigned long long total = boot_time + starttime;
    time_t aa = (total);
    time_struct_to_string(buf, 20, localtime(&aa));
    return buf;
}
char* calc_time_str(unsigned long utime, unsigned long stime){
    utime = utime / sysconf(_SC_CLK_TCK);
    stime = stime / sysconf(_SC_CLK_TCK);
    unsigned long long total = utime + stime;
    size_t seconds = total % 60;
    size_t minutes = total / 60;
    char* buf = malloc(20);
    execution_time_to_string(buf, 20, minutes, seconds);
    return buf;
}

int redirect_output(char* command){
    char* loc = strstr(command, " > ");
    *loc = '\0';
    char* filename = malloc(strlen(loc + 3) + 1);
    strcpy(filename, loc + 3);
    char** args = getargs(command);
    FILE* file = fopen(filename, "w");
    if(file == NULL){
         print_redirection_file_error();
         return 1;
    }
    fflush(stdout);
    fflush(stdin);
    fflush(file);
    pid_t f = fork();
    if(f == -1){
        size_t i = 0;
        while(args[i] != NULL){
            free(args[i]);
            i++;
        }
        free(args[i]);
        free(filename);
        fclose(file);
        return 1;
    }
    //child
    else if(f == 0){
        print_command_executed(getpid());
        fflush(stdout);
        fflush(stdin);
        fflush(file);
        dup2(fileno(file), STDOUT_FILENO);
        fclose(file);
        free(filename);
        execvp(args[0], args);
        print_exec_failed(command);
        print_invalid_command(command);
        size_t i = 0;
        while(args[i] != NULL){
            free(args[i]);
            i++;
        }
        free(args);
        cleanup();
        exit(1);
    }
    else{
        foreground = f;
        size_t i = 0;
        while(args[i] != NULL){
            free(args[i]);
            i++;
        }
        free(args);
        free(filename);
        int status;
        int res = waitpid(f, &status, 0);
        if(res == -1){
            print_wait_failed();
        }
        foreground = 0;
        fclose(file);
        return 0;
    }
}

int redirect_append(char* command){
    char* loc = strstr(command, " >> ");
    *loc = '\0';
    char* filename = malloc(strlen(loc + 4) + 1);
    strcpy(filename, loc + 4);
    char** args = getargs(command);
    FILE* file = fopen(filename, "a");
    if(file == NULL){
         print_redirection_file_error();
         return 1;
    }
    fflush(stdout);
    fflush(stdin);
    fflush(file);
    pid_t f = fork();
    if(f == -1){
        size_t i = 0;
        while(args[i] != NULL){
            free(args[i]);
            i++;
        }
        free(args[i]);
        free(filename);
        fclose(file);
        return 1;
    }
    //child
    else if(f == 0){
        print_command_executed(getpid());
        fflush(stdout);
        fflush(stdin);
        fflush(file);
        dup2(fileno(file), STDOUT_FILENO);
        fclose(file);
        free(filename);
        execvp(args[0], args);
        print_exec_failed(command);
        print_invalid_command(command);
        size_t i = 0;
        while(args[i] != NULL){
            free(args[i]);
            i++;
        }
        free(args);
        cleanup();
        exit(1);
    }
    else{
        foreground = f;
        size_t i = 0;
        while(args[i] != NULL){
            free(args[i]);
            i++;
        }
        free(args);
        free(filename);
        int status;
        int res = waitpid(f, &status, 0);
        if(res == -1){
            print_wait_failed();
        }
        foreground = 0;
        fclose(file);
        return 0;
    }
}

int redirect_input(char* command){
    char* loc = strstr(command, " < ");
    *loc = '\0';
    char* filename = malloc(strlen(loc + 3) + 1);
    strcpy(filename, loc + 3);
    char** args = getargs(command);
    printf("filename: %s\n", filename);
    FILE* file = fopen(filename, "r");
    if(file == NULL){
         print_redirection_file_error();
         return 1;
    }
    fflush(stdout);
    fflush(stdin);
    fflush(file);
    pid_t f = fork();
    if(f == -1){
        size_t i = 0;
        while(args[i] != NULL){
            free(args[i]);
            i++;
        }
        free(args[i]);
        free(filename);
        fclose(file);
        return 1;
    }
    //child
    else if(f == 0){
        print_command_executed(getpid());
        fflush(stdout);
        fflush(stdin);
        fflush(file);
        dup2(fileno(file), STDIN_FILENO);
        fclose(file);
        free(filename);
        execvp(args[0], args);
        print_exec_failed(command);
        print_invalid_command(command);
        size_t i = 0;
        while(args[i] != NULL){
            free(args[i]);
            i++;
        }
        free(args);
        cleanup();
        exit(1);
    }
    else{
        foreground = f;
        size_t i = 0;
        while(args[i] != NULL){
            free(args[i]);
            i++;
        }
        free(args);
        free(filename);
        int status;
        int res = waitpid(f, &status, 0);
        if(res == -1){
            print_wait_failed();
        }
        foreground = 0;
        fclose(file);
        return 0;
    }
}