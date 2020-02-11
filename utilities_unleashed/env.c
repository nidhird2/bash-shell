/**
 * Utilities Unleashed
 * CS 241 - Spring 2020
 * partner: slj2
 */
#include "format.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int loc = -1;
    for(int i = 1; i < argc; i++){
        if(strcmp("--", argv[i]) == 0){
            loc = i;
            break;
        }
    }
    if(loc == (argc - 1) || loc == -1){
        print_env_usage();
    }

    pid_t pid = fork();
    if(pid == -1){
        print_fork_failed();
        exit(1);
    }
    else if(pid != 0){
        int status;
        waitpid(pid, &status, 0);
        if(status != 0 || status != 23){
            return 1;
        }
        for(int i = 0; i < loc; i++){
            char * tok = strtok(argv[i], "=");
            if(tok == NULL){
                continue;
            }
            unsetenv(tok);
        }
        exit(0);
    }
    for(int i = 1; i < loc; i++){
        char * tok = strtok(argv[i], "=");
        char* tok2 = strtok(NULL, "");
        int res;
        if(strlen(tok2) >= 1 && tok2[0] == '%'){
            char* envToFind = tok2 + 1;
            char* found = getenv(envToFind);
            if(found == NULL){
                print_env_usage();
                exit(23);
            }
            res = setenv(tok, found, 0);
        }
        else {
            res = setenv(tok, tok2, 0);
        }
        if(res == -1){
            print_env_usage();
            exit(23);
        }
    }
    execvp(argv[loc + 1], argv + loc + 1);
    print_exec_failed();
    exit(1);
}
