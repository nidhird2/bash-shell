/**
 * Mini Memcheck
 * CS 241 - Spring 2020
 */
#include "mini_memcheck.h"
#include <stdio.h>
#include <string.h>
#include <stdio.h>

size_t total_memory_requested;
size_t total_memory_freed;
size_t invalid_addresses;
meta_data *head;

void *mini_malloc(size_t request_size, const char *filename,
                  void *instruction) {
    // your code here
    setvbuf(stdout, NULL, _IONBF, 0);
    // printf("mem reque sted: %lu\n", request_size);
    if(request_size == 0){
        return NULL;
    }
    void* result = malloc(request_size + sizeof(meta_data));
    if(result == NULL){
        return NULL;
    }
    //printf("result: %p\n", result);
    ((meta_data*)result)-> request_size = request_size;
    ((meta_data*)result)-> filename = filename;
    ((meta_data*)result)-> instruction = instruction;
    ((meta_data*)result)-> next = head;
    head = (meta_data*)result;
    void* pt2 = ((meta_data*)result) + 1;
    total_memory_requested += request_size;
    // printf("total malloced: %lu\n", request_size + sizeof(meta_data));
    // printf("head: %p\n", head);
    // printf("pt2: %p\n", pt2);
    // printf("head+1: %p\n", head+1);
    // printf("total mem requested: %lu\n", total_memory_requested);
    return pt2;
}

void *mini_calloc(size_t num_elements, size_t element_size,
                  const char *filename, void *instruction) {
    // your code here
    if(num_elements == 0 || element_size == 0){
        return NULL;
    }
    size_t total_size = num_elements * element_size;
    void* result = mini_malloc(total_size, filename, instruction);
    if(result == NULL){
        return NULL;
    }
    else{
        memset(result, 0, total_size);
        return result;
    }
}

void *mini_realloc(void *payload, size_t request_size, const char *filename,
                   void *instruction) {
    // your code here  
    setvbuf(stdout, NULL, _IONBF, 0);
    if(payload == NULL){
        return mini_malloc(request_size, filename, instruction);
    }
    else if(request_size == 0){
        mini_free(payload);
    }
    if(head + 1 == payload){
        size_t old = head->request_size;
        meta_data* result = realloc(head, request_size + sizeof(meta_data));
        //realloc failed
        if(result == NULL){
            return NULL;
        }
        ((meta_data*)result)->request_size = request_size;
        ((meta_data*)result)->filename = filename;
        ((meta_data*)result)->instruction = instruction;
        head = result;
        if(old < request_size){
            total_memory_requested += (request_size - old);
        } else {
            total_memory_freed += (old - request_size);
        }
        return result + 1;
    }
    meta_data* current = head;
    while(current ->next != NULL){
        if((current->next) + 1 == payload){
            size_t old = current->next->request_size;
            meta_data* result = realloc(current->next, request_size + sizeof(meta_data));
            if(result == NULL){
                return NULL;
            }
            current->next = result;
            ((meta_data*)result)->request_size = request_size;
            ((meta_data*)result)->filename = filename;
            ((meta_data*)result)->instruction = instruction;
            if(old < request_size){
            total_memory_requested += (request_size - old);
            } else {
            total_memory_freed += (old - request_size);
            }
            return result + 1;
        }
    }
    //no match found
    invalid_addresses++;
    return NULL;
}

void mini_free(void *payload) {
    // your code here
    setvbuf(stdout, NULL, _IONBF, 0);
    if(payload == NULL){
        return;
    }
    meta_data* remove;
    if(head + 1 == payload){
        remove = head;
        head = head->next;
        total_memory_freed += remove->request_size;
        free(remove);
        return;
    }
    meta_data* current = head;
    while(current != NULL){
        if((current ->next) + 1 == payload){
            remove = current->next;
            if(remove->next == NULL){
                current->next = NULL;
            }
            else{
                current->next = remove->next;
            }
            total_memory_freed += remove->request_size;
            free(remove);
            return;
        }
        current = current->next;
    }
    invalid_addresses++;
    return;
}
