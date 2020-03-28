/**
 * Parallel Make
 * CS 241 - Spring 2020
 */

#include "format.h"
#include "graph.h"
#include "queue.h"
#include "parmake.h"
#include "parser.h"
#include "set.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>



graph* g;
queue* q;
int FAILED = -1;
int COMPLETED = 1;
int INCOMPLETE = 0;

void cleanup(){
    queue_destroy(q);
    graph_destroy(g);
}

void print_str_vector(vector* this){
    size_t vl = vector_size(this);
    for(size_t i = 0; i < vl; i++){
        printf("%lu: %s\n", i, (char*)vector_get(this, i));
    }
}

int recursor(char* target, set* visited, set* recStack,vector* commands){
    if(!set_contains(visited, target)){
        set_add(visited, target);
        set_add(recStack, target);
        vector* dependencies = graph_neighbors(g, target);
        size_t dep_length = vector_size(dependencies);
        for(size_t i = 0; i < dep_length; i++){
            char* current = (char*)vector_get(dependencies, i);
            if(!set_contains(visited, current) && recursor(current, visited, recStack, commands)){
                vector_destroy(dependencies);
                return 1;
            }   
            else if(set_contains(recStack, current)){
                vector_destroy(dependencies);
                return 1;
            }
        }   
        vector_destroy(dependencies);
        vector_push_back(commands, target);
    }
    set_remove(recStack, target);
    return 0;
}

vector* get_command_list(char* target){
    //vector* all_targets = graph_vertices(g);
    vector* commands = string_vector_create();
    set* visited = string_set_create();
    set* recStack = string_set_create();
    //set_add(visited, target);
    if(recursor(target, visited, recStack, commands)){
        //printf("CYCLE FOUND\n");
        //print_str_vector(commands);
        vector_destroy(commands);
        set_destroy(visited);
        set_destroy(recStack);
        return NULL;
    }
    set_destroy(visited);
    set_destroy(recStack);
    return commands;
}

void run_commands(rule_t* rule){
    for(size_t j = 0; j < vector_size(rule->commands); j++){
        int return_val = system(vector_get(rule->commands, j));
        //if command failed
        if(return_val != 0){
            rule->state = FAILED;
            return;
        }
    }
    if(rule->state != FAILED){
        rule->state = COMPLETED;
    }
}

int check_failed_children(rule_t* rule, vector* dependencies){
    if(rule->state != INCOMPLETE){
        return 0;
    }
    for(size_t k = 0; k < vector_size(dependencies); k++){
        rule_t * rule_k = (rule_t*)graph_get_vertex_value(g, vector_get(dependencies, k));
        if(rule_k->state == FAILED){
            return 1;
        }
    }
    return 0;
}

int check_modf_times(rule_t * rule, vector* dependencies){
    struct stat current_info;
    stat(rule->target, &current_info);
    for(size_t j = 0; j < vector_size(dependencies); j++){
        struct stat dep_info;
        stat(vector_get(dependencies, j), &dep_info);
        // printf("dep time: %s\n", asctime(gmtime(&dep_info.st_mtime)));
        // printf("rule time: %s\n", asctime(gmtime(&current_info.st_mtime)));
        // printf("time diff: %f\n", diff);
        if(difftime(dep_info.st_mtime, current_info.st_mtime) > 1.0){
            return 1;
        }
    }
    rule->state = 1;
    return 0;
}

void* compute(void* input){
    char* target = (char*)input;
    vector* targets = get_command_list(target);
    if(targets == NULL){
        print_cycle_failure(target);
        return NULL;
    }
    //printf("input: %s\n", target);
    for(size_t i = 0; i < vector_size(targets); i++){
        char* current = (char*)vector_get(targets, i);
        rule_t * rule = (rule_t *)graph_get_vertex_value(g, current);
        //printf("rule target: %s\n", current);
        //check if rule has already been processed
        if(rule->state != INCOMPLETE){
            continue;
        }
        vector* dependencies = graph_neighbors(g, current);
        //check if depedencies failed
        if(check_failed_children(rule, dependencies)){
            rule->state = FAILED;
            vector_destroy(dependencies);
            continue;
        }
        //if target is not on disk, run commands
        if(access(current, F_OK) != 0){
            run_commands(rule);
        }
        else{
            int run_all_commands = 0;
            //otherwise target must be on disk
            for(size_t j = 0; j < vector_size(dependencies); j++){
                //check if rule depends on something NOT on disk
                if(access(vector_get(dependencies, j), F_OK) != 0){
                    run_all_commands = 1;
                    break;
                }
            }
            //if all dependencies are files on disk, check modf. times
            if(run_all_commands == 0){
                run_all_commands = check_modf_times(rule, dependencies);
            }
            else{
                run_commands(rule);
            }
        }
        vector_destroy(dependencies);
    }
    vector_destroy(targets);
    return NULL;
}

// char* get_first_target(char* makefile){
//     vector* neighbors = graph_neighbors(g, "");
//     char* result = (char*)vector_pop_back(neighbors);
//     free(neighbors);
//     return result;
// }

int parmake(char *makefile, size_t num_threads, char **targets) {
    // good luck!
    g = parser_parse_makefile(makefile, targets);
    q = queue_create(-1);
    //graph_setup();
    //if targets is NULL or empty, find first
    if(targets == NULL || targets[0] == NULL){
        vector* neighbors = graph_neighbors(g, "");
        char* target = vector_get(neighbors, 0);
        compute(target);
        vector_destroy(neighbors);
        cleanup();
        return 0;
    }
    else{
        int i = 0;
        char* current = targets[i];
        while(current != NULL){
            queue_push(q, current);
            i++;
            current = targets[i];
        }
    }
    queue_push(q, NULL);
    void* t = queue_pull(q);
    while(t != NULL){
        compute(t);
        t = queue_pull(q);
    }
    cleanup();
    return 0;
}
