/**
 * Teaching Threads
 * CS 241 - Spring 2020
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "reduce.h"
#include "reducers.h"

/* You might need a struct for each task ... */
typedef struct thread_task{
    //size_t start_idx;
    //size_t end_idx;
    int* list;
    reducer func;
    int base_case;
    size_t length;
} task_t;

void* reduce_thread(void* input){
    task_t* task = (task_t*)input;
    //size_t length = task->end_idx -task->start_idx + 1;
    //printf("start_idx: %lu, end_idx: %lu, length: %lu\n", task->start_idx, task->end_idx, length);
    //int l[length];
    // for(size_t j = task->start_idx; j <= task->end_idx; j++){
    //     l[j - task->start_idx] = task->list[j];
    // }
    int* real_result = (int*)malloc(sizeof(int));
    //*real_result = reduce(task->list, , task->func, task->base_case);
    *real_result = reduce(task->list, task->length, task->func, task->base_case);
    //*real_result = 0;
    return real_result;
}

/* You should create a start routine for your threads. */

int par_reduce(int *list, size_t list_len, reducer reduce_func, int base_case,
               size_t num_threads) {
    /* Your implementation goes here */
    if(num_threads == 1){
        return reduce(list, list_len, reduce_func, base_case);
    }
    size_t new_length;
    size_t max_use_threads = list_len / 2;
    if(num_threads > max_use_threads){
        new_length = max_use_threads;
    }
    else{
        new_length = num_threads;
    }
    int new_list[new_length];
    pthread_t tids[new_length];
    task_t tasks[new_length];
    size_t each_size = list_len / new_length;
    //need to divide tasks to threads
    for(size_t i = 0; i < new_length; i++){
        tasks[i].base_case = base_case;
        tasks[i].func = reduce_func;
        tasks[i].list = list + i * each_size;
        if(i + 1 != new_length){
            //tasks[i].start_idx= i * each_size;
            //tasks[i].end_idx = tasks[i].start_idx + each_size;
            tasks[i].length = each_size;
        } else{
            //tasks[i].start_idx = i * each_size;
            //tasks[i].end_idx = list_len - 1;
            tasks[i].length = each_size + (list_len % new_length);
        }
        //printf("start_idx: %lu, end_idx: %lu\n", tasks[i].start_idx, tasks[i].end_idx);
        //printf("new length: %lu\n", tasks[i].end_idx -tasks[i].start_idx + 1);
        pthread_create(tids + i, NULL, reduce_thread, (void*)(tasks + i));
    }
    //join thread results & store in new list
    for(size_t i = 0; i < new_length; i++){
        void* result;
        pthread_join(tids[i], &result);
        new_list[i] = *(int*)result;
        free(result);
    }

    //combine thread results by reducing as normal
    return reduce(new_list, new_length, reduce_func, base_case);
}
