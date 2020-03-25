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
#include <sys/wait.h>

graph* g;
queue* q;

void cleanup(){
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
                free(dependencies);
                return 1;
            }   
            else if(set_contains(recStack, current)){
                free(dependencies);
                return 1;
            }
        }   
        free(dependencies);
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
        return NULL;
    }
    set_destroy(visited);
    return commands;
}



void* compute(void* input){
    char* target = queue_pull(q);
    vector* targets = get_command_list(target);
    if(targets == NULL){
        print_cycle_failure(target);
        return NULL;
    }
    for(size_t i = 0; i < vector_size(targets); i++){
        char* current = (char*)vector_get(targets, i);
        //printf("%lu: %s\n", i, current);
        rule_t * rule = (rule_t *)graph_get_vertex_value(g, current);
        //if rule's child dependencies have failed
        if(rule->state == -1){
            vector* antin = graph_antineighbors(g, current);
            for(size_t k = 0; k < vector_size(antin); k++){
                    rule_t * rule_k = (rule_t*)graph_get_vertex_value(g, vector_get(antin, k));
                    rule_k->state = -1;
            }
            break;
        }
        for(size_t j = 0; j < vector_size(rule->commands); j++){
            int return_val = system(vector_get(rule->commands, j));
            //printf("return val: %d\n", return_val);
            if(return_val != 0){
                rule->state = 1;
                vector* antin = graph_antineighbors(g, current);
                //mark parents as failed
                for(size_t k = 0; k < vector_size(antin); k++){
                    rule_t * rule_k = (rule_t*)graph_get_vertex_value(g, vector_get(antin, k));
                    rule_k->state = -1;
                }
                break;
            }
        }
    }
    free(targets);
    return NULL;
}

int parmake(char *makefile, size_t num_threads, char **targets) {
    // good luck!
    g = parser_parse_makefile(makefile, targets);
    q = queue_create(-1);
    //graph_setup();
    int i = 0;
    char* current = targets[i];
    while(current != NULL){
        queue_push(q, current);
        i++;
        current = targets[i];
    }
    queue_push(q, NULL);
    compute(NULL);
    cleanup();
    return 0;
}
