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
#include <pthread.h>

void compute(rule_t*);

set* s_rules;
vector* v_rules;
graph* g;
queue* q;
//queue* rule_order;
int FAILED = -10;
int COMPLETED = -2;
//int INCOMPLETE = -3;
int INPROGRESS = -1;
int READY = 0;
pthread_cond_t cv;
pthread_mutex_t m;
// pthread_mutex_t graph_lock;
// static pthread_barrier_t setup;
// static pthread_barrier_t computing;

void cleanup(){
    queue_destroy(q);
    //queue_destroy(order);
    graph_destroy(g);
    vector_destroy(v_rules);
    set_destroy(s_rules);
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

vector* get_dependencies(char* target){
    vector* commands = string_vector_create();
    set* visited = string_set_create();
    set* recStack = string_set_create();
    recursor(target, visited, recStack, commands);
    set_destroy(recStack);
    set_destroy(visited);
    return commands;
}

void get_all_rules(void){
    char* target = (char*)queue_pull(q);
    while(target != NULL){
        vector* commands = string_vector_create();
        set* visited = string_set_create();
        set* recStack = string_set_create();
        //set_add(visited, target);
        if(recursor(target, visited, recStack, commands)){
            print_cycle_failure(target);
        }
        else{
            for(size_t i = 0; i < vector_size(commands); i++){
                set_add(s_rules, graph_get_vertex_value(g, vector_get(commands, i)));
            }
        }
        set_destroy(visited);
        set_destroy(recStack);
        vector_destroy(commands);
        target = (char*)queue_pull(q);
    }
    return;
}

void remove_uneccesary_vertexes(){
    vector* unused_commands = graph_vertices(g);
    for(size_t i = 0; i < vector_size(unused_commands); i++){
        rule_t* rule = graph_get_vertex_value(g, vector_get(unused_commands, i));
        if(!set_contains(s_rules, rule)){
            graph_remove_vertex(g, rule->target);
        }
    }
    vector_destroy(unused_commands);
}

void top_sort_util(char* current, set* visited, vector* stack){
    set_add(visited, current);

    vector* neigh = graph_neighbors(g, current);
    for(size_t i = 0; i < vector_size(neigh); i++){
        char* n = (char*)vector_get(neigh, i);
        if(!set_contains(visited, n)){
            top_sort_util(n, visited, stack);
        }
    }
    vector_insert(stack, 0, current);
    vector_destroy(neigh);
}

void get_order(queue* q){
    vector* all_v = graph_vertices(g);
    set* visited = shallow_set_create();
    vector* stack = shallow_vector_create();

    for(size_t i = 0; i < vector_size(all_v); i++){
        char* current = (char*)vector_get(all_v, i);
        if(!set_contains(visited, current)){
            top_sort_util(current, visited, stack);
        }
    }
    while(!vector_empty(stack)){
        rule_t* rule = graph_get_vertex_value(g, vector_get(stack, 0));
        vector* n = graph_neighbors(g, rule->target);
        rule->state = (int)vector_size(n);
        vector_destroy(n);
        vector_push_back(v_rules, rule);
        queue_push(q, rule);
        vector_erase(stack, 0);
    }

    vector_destroy(all_v);
    set_destroy(visited);
    vector_destroy(stack);
}

int run_commands(rule_t* rule){
    for(size_t j = 0; j < vector_size(rule->commands); j++){
        int return_val = system(vector_get(rule->commands, j));
        //if command failed
        if(return_val != 0){
            return FAILED;
        }
    }
    return COMPLETED;
}

int check_failed_children(vector* dependencies){
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
        if(difftime(dep_info.st_mtime, current_info.st_mtime) >= 1){
            return 1;
        }
    }
    return 0;
}

rule_t* find_rule(){
    for(size_t i = 0; i < vector_size(v_rules); i++){
            rule_t* current = (rule_t*)vector_get(v_rules, i);
            if(current->state == READY){
                vector_erase(v_rules, i);
                return current;
            }
    }
    return NULL;
}


void* thread(void* inp){
    pthread_mutex_lock(&m);
    //printf("v rules size: %lu\n", vector_size(v_rules));
    while(vector_size(v_rules) != 0){
        rule_t* rule = find_rule();
        if(rule != NULL){
            //printf("tid: %lu computing target: %s\n", pthread_self(), rule->target);
            rule->state = INPROGRESS;
            compute(rule);
        }
        else{
            pthread_cond_wait(&cv, &m);
        }
    }
    pthread_mutex_unlock(&m);
    return NULL;
}


void compute(rule_t* rule){
    char* current = rule->target;
    vector* dependencies = graph_neighbors(g, current);
    int result_state = COMPLETED;
    //check if depedencies failed
    if(check_failed_children(dependencies)){
        result_state = FAILED;
        pthread_mutex_unlock(&m);
    }
    else{
        pthread_mutex_unlock(&m);
        //if rule name is not file on disk, run commands
        if(access(rule->target, F_OK) != 0){
            result_state = run_commands(rule);
        }
        else{
            int run_all_commands = 0;
            pthread_mutex_lock(&m);
            vector* all_dependencies = get_dependencies(current);
            pthread_mutex_unlock(&m);
            //remove current rule from list
            vector_pop_back(all_dependencies);
            //otherwise target must be on disk
            for(size_t j = 0; j < vector_size(all_dependencies); j++){
                //check if rule depends on something NOT on disk
                if(access(vector_get(all_dependencies, j), F_OK) != 0){
                    run_all_commands = 1;
                    break;
                }
            }
            //if all dependencies are files on disk, check modf. times
            if(run_all_commands == 0){
                run_all_commands = check_modf_times(rule, all_dependencies);
            }
            //run commands or mark as completed
            if(run_all_commands){
                result_state = run_commands(rule);
            }
            else{
                result_state = COMPLETED;
            }
            vector_destroy(all_dependencies);
        }
    }
    //printf("target: %s result state found!\n", rule->target);
    vector_destroy(dependencies);
    pthread_mutex_lock(&m);
    rule->state = result_state;
    vector* anti_n = graph_antineighbors(g, rule->target);
    for(size_t i = 0; i < vector_size(anti_n); i++){
        rule_t* current = (rule_t*)graph_get_vertex_value(g, vector_get(anti_n, i));
        current->state--;
    }
    pthread_cond_broadcast(&cv);
    return;
}

int parmake(char *makefile, size_t num_threads, char **targets) {
    // good luck!
    //printf("num threads: %lu\n", num_threads);
    g = parser_parse_makefile(makefile, targets);
    q = queue_create(-1);
    s_rules = shallow_set_create();
    v_rules = shallow_vector_create();
    pthread_mutex_init(&m, NULL);
    pthread_cond_init(&cv, NULL);
    //graph_setup();
    //if targets is NULL or empty, find first
    if(targets == NULL || targets[0] == NULL){
        vector* neighbors = graph_neighbors(g, "");
        char* target = vector_get(neighbors, 0);
        queue_push(q, target);
        vector_destroy(neighbors);
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
    get_all_rules();
    remove_uneccesary_vertexes();
    get_order(q);

    pthread_t tids[num_threads];
    for(size_t i = 0; i < num_threads; i++){
        pthread_create(tids + i, NULL, thread, NULL);
    }

    for(size_t i = 0; i < num_threads; i++){
        void* result;
        pthread_join(tids[i], &result);
    }

    cleanup();
    return 0;
}
