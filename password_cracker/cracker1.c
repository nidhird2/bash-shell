/**
 * Password Cracker
 * CS 241 - Spring 2020
 */
#include "cracker1.h"
#include <crypt.h>
#include "format.h"
#include "utils.h"
#include <stdio.h>
#include "includes/queue.h"
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

//#define USERNAME_LENGTH = 8;


static queue* tasks;
static int total_tasks = 0;
//static int tasks_left = 0;
int stop = 0;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

void cleanup(){
    queue_destroy(tasks);
}

int crack_one(char* crypt, struct crypt_data* cdata, int thread_idx){
    // printf("crypt: %s\n", crypt);
    char username[9];
    char hash[14];
    char known_pass[9];
    int hash_count = 0;
    sscanf(crypt, "%s %s %s", username, hash, known_pass);
    free(crypt);
    double before_time = getThreadCPUTime();
    v1_print_thread_start(thread_idx, username);
    int known_pass_chars = getPrefixLength(known_pass);
    int password_length = strlen(known_pass);
    //char* loc = strstr(known_pass, ".");
    if(known_pass_chars == password_length){
        double time_elp = getThreadCPUTime() - before_time;
        v1_print_thread_result(thread_idx, username, known_pass, hash_count, time_elp, 0);
        return 1;
    }
    int unknown_chars = password_length - known_pass_chars;
    char* pass_guess = (char*)malloc(9);
    //size_t offset = 0;
    //copys known part of password into guess
    memcpy(pass_guess, known_pass, known_pass_chars);
    for(int i = known_pass_chars; i < password_length; i++){
        pass_guess[i] = 'a';
    }
    pass_guess[password_length] = '\0';
    //starts iterations
    long total_it = (long)pow(26, unknown_chars);
    printf("total_it: %ld\n", total_it);
    printf("pass guess: %s\n", pass_guess);
    const char* salt = "xx";
    for(long i = 0; i < total_it; i++){
        char* result_hash = crypt_r(pass_guess, salt, cdata);
        hash_count++;
        if(strcmp(hash, result_hash) == 0){
            double time_elp = getThreadCPUTime() - before_time;
            v1_print_thread_result(thread_idx, username, pass_guess, hash_count, time_elp, 0);
            free(pass_guess);
            return 1;
        }
        incrementString(pass_guess);        
    }
    //pass not found
    double time_elp = getThreadCPUTime() - before_time;
    v1_print_thread_result(thread_idx, username, pass_guess, hash_count, time_elp, 1);
    free(pass_guess);
    return 0;
}

void* f1(void* ptr){
    int thread_idx = *(int*)ptr;
    int* count = (int*)calloc(1, sizeof(int));
    struct crypt_data cdata;
    cdata.initialized = 0;
    char* crypt = NULL;
    pthread_mutex_lock(&m);
    if(!stop){
        crypt = (char*)queue_pull(tasks);
    }
    if(crypt == NULL){
        stop = 1;
    }
    while(!stop){
        pthread_mutex_unlock(&m);
        *count += crack_one(crypt, &cdata, thread_idx);
        pthread_mutex_lock(&m);
        if(!stop){
            crypt = queue_pull(tasks);
        }
        if(crypt == NULL){
            stop = 1;
        }
    }
    pthread_mutex_unlock(&m);
    return (void*)count;
    }

void print_queue_contents(){
    char* top = (char*)queue_pull(tasks);
    while(top){
        printf("%s\n", top);
        top = (char*)queue_pull(tasks);
    }
}

void populate_queue(){
    tasks = queue_create(-1);
    char * line = NULL;
    size_t len = 0;
    int read = getline(&line, &len, stdin);
    while (read != -1) {
        if(line[read - 1] == '\n'){
            line[read - 1] = '\0';
        }
        queue_push(tasks, strdup(line));
        total_tasks++;
        read = getline(&line, &len, stdin);
    }
    free(line);
    queue_push(tasks, NULL);
    return;
}

int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads
    populate_queue();

    pthread_t tids[thread_count];
    int idxs[thread_count];
    for(size_t i = 0; i < thread_count; i++){
        idxs[i] = i + 1;
        pthread_create(tids + i, NULL, f1, (void*)(idxs + i));
    }
    int num_found = 0;
    for(size_t i = 0; i < thread_count; i++){
        void* result;
        pthread_join(tids[i], &result);
        num_found += *(int*)result;
        free(result);
    }
    //print_queue_contents();
    // struct crypt_data cdata;
    // cdata.initialized = 0;
    v1_print_summary(num_found, total_tasks - num_found);
    cleanup();
    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}
