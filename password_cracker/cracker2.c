/**
 * Password Cracker
 * CS 241 - Spring 2020
 */
#include "cracker2.h"
#include "format.h"
#include "utils.h"
#include <crypt.h>
#include "includes/queue.h"
#include "string.h"
#include "unistd.h"
#include "stdio.h"
#include <pthread.h>
#include <thread_status.h>

//static barrier_t barrier;

static size_t num_threads = 0;
//pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
//pthread_cond_t *cv_return;
//pthread_cond_t *cv_wait;
//this mutex locks return vals from task
pthread_mutex_t m_return;
pthread_mutex_t m_wait;
static pthread_barrier_t setup;
static pthread_barrier_t computing;
//static pthread_barrier_t b;
static queue* tasks;
static int total_tasks = 0;
//static int current_task_no = 0;
//things needed to complete task
static char* username = NULL;
static char* hash = NULL;
static char* known_pass = NULL;
//things needed to be updated after task
static char* found_pass = NULL;
static int total_hash_count = 0;
static int found = 0;

//sync threads
int task_waiting = 0;
int stopAll = 0;

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

void cleanup(){
    queue_destroy(tasks);
    pthread_barrier_destroy(&setup);
    pthread_barrier_destroy(&computing);
    free(username);
    free(hash);
    free(known_pass);
}

void print_queue_contents(){
    char* top = (char*)queue_pull(tasks);
    while(top){
        printf("%s\n", top);
        top = (char*)queue_pull(tasks);
    }
}

//returns hash count
void crack_part(int thread_id, struct crypt_data* cdata){
    //threadStatusSet("cracking");
    int known_pass_chars = getPrefixLength(known_pass);
    int password_length = strlen(known_pass);
    int unknown_chars = password_length - known_pass_chars;
    int hash_count = 0;
    char* pass_guess = (char*)malloc(9);
    memset(pass_guess, 'a', password_length);
    pass_guess[password_length] = '\0';
    long start_idx;
    long count;

    const char* salt = "xx";
    
    getSubrange(unknown_chars, num_threads, thread_id, &start_idx, &count);
    setStringPosition(pass_guess, start_idx);
    memcpy(pass_guess, known_pass, known_pass_chars);
    v2_print_thread_start(thread_id, username, start_idx, pass_guess);
    for(long i = 0; i < count; i++){
        pthread_mutex_lock(&m_return);
        if(found){
            free(pass_guess);
            total_hash_count += hash_count;
            v2_print_thread_result(thread_id, hash_count, 1);
            pthread_mutex_unlock(&m_return);
            return;
        }
        pthread_mutex_unlock(&m_return);
        char* result_hash = crypt_r(pass_guess, salt, cdata);
        hash_count++;
        if(strcmp(hash, result_hash) == 0){
            pthread_mutex_lock(&m_return);
            total_hash_count += hash_count;
            found = 1;
            found_pass = pass_guess;
            v2_print_thread_result(thread_id, hash_count, 0);
            pthread_mutex_unlock(&m_return);
            return;
        }
        incrementString(pass_guess); 
    }
    free(pass_guess);
    pthread_mutex_lock(&m_return);
    total_hash_count += hash_count;
    v2_print_thread_result(thread_id, hash_count, 2);
    pthread_mutex_unlock(&m_return);
    return;
}

void outer(){

}

void* f1(void* ptr){
    int my_thread_id = *(int*)ptr;
    struct crypt_data cdata;
    cdata.initialized = 0;
    //pthread_mutex_lock(&m_wait);
    while(1){
        //threadStatusSet("top of loop");
        //pthread_mutex_unlock(&m_wait);
        //threadStatusSet("mtx unlocked");
        pthread_barrier_wait(&setup);
        // while(!task_waiting && !stopAll){
        //     pthread_cond_wait(&cv, &m_wait);
        // }
        //threadStatusSet("waiting for mtx");
        pthread_mutex_lock(&m_wait);
        //threadStatusSet("mtx aquired");
        if(stopAll){
            pthread_mutex_unlock(&m_wait);
            return NULL;
        }
        pthread_mutex_unlock(&m_wait);
        crack_part(my_thread_id, &cdata);
        //task_waiting = 0;
        pthread_barrier_wait(&computing);
        // threadStatusSet("waiting for mtx");
        // threadStatusSet("mtx aquired");
    }
    //threadStatusSet("trying to unlock");
    pthread_mutex_unlock(&m_wait);
    //threadStatusSet("unlock success");
    return NULL;
}


int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads
    num_threads = thread_count;
    //pthread_barrier_init(&b, NULL, thread_count);
    pthread_barrier_init(&setup, NULL, thread_count + 1);
    pthread_barrier_init(&computing, NULL, thread_count + 1);
    pthread_mutex_init(&m_wait, NULL);
    pthread_mutex_init(&m_return, NULL);
    username = (char*)malloc(9);
    hash = (char*)malloc(14);
    known_pass = (char*)malloc(9);

    //get threads going
    pthread_t tids[thread_count];
    int idxs[thread_count];
    for(size_t i = 0; i < thread_count; i++){
        idxs[i] = i + 1;
        pthread_create(tids + i, NULL, f1, (void*)(idxs + i));
    }
    //read all tasks
    populate_queue();



    //processing tasks
    //pthread_mutex_lock(&m_wait);
    char* crypt = queue_pull(tasks);
    while(crypt != NULL){
        pthread_mutex_lock(&m_wait);
        sscanf(crypt, "%s %s %s", username, hash, known_pass);
        pthread_mutex_unlock(&m_wait);
        free(crypt); 
        //task_waiting = 1;
        v2_print_start_user(username);
        double start_time = getTime();
        double start_cpu = getCPUTime();
        //pthread_cond_broadcast(&cv);
        //pthread_mutex_lock(&m_wait);
        // while(task_waiting){
        //     pthread_cond_wait(&cv, &m_wait);
        // }
        pthread_barrier_wait(&setup);
        pthread_barrier_wait(&computing);
        double elp_time = getTime()-start_time;
        double cpu_time = getCPUTime() - start_cpu;
        //printf("username: %s\n", username);
        //printf("username: %s\n", username);
        int result = 1;
        if(found){
            result = 0;
        }
        v2_print_summary(username, found_pass, total_hash_count, elp_time, cpu_time, result);
        total_hash_count = 0;
        found = 0;
        free(found_pass);
        found_pass = NULL;
        crypt = queue_pull(tasks);
        //printf("crypt: %s\n", crypt);
    }
    //set flag to stop all threads
    stopAll = 1;
    //pthread_mutex_unlock(&m_wait);
    pthread_barrier_wait(&setup);
    //pthread_cond_broadcast(&cv);
    //join all threads
    
    for(size_t i = 0; i < thread_count; i++){
        void* result;
        pthread_join(tids[i], &result);
    }
    cleanup();
    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}
