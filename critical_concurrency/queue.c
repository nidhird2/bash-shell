/**
 * Critical Concurrency
 * CS 241 - Spring 2020
 */
#include "queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * This queue is implemented with a linked list of queue_nodes.
 */
typedef struct queue_node {
    void *data;
    struct queue_node *next;
} queue_node;

struct queue {
    /* queue_node pointers to the head and tail of the queue */
    queue_node *head, *tail;

    /* The number of elements in the queue */
    ssize_t size;

    /**
     * The maximum number of elements the queue can hold.
     * max_size is non-positive if the queue does not have a max size.
     */
    ssize_t max_size;

    /* Mutex and Condition Variable for thread-safety */
    pthread_cond_t cv;
    pthread_mutex_t m;
};

queue *queue_create(ssize_t max_size) {
    /* Your code here */
    queue* result = (queue*)malloc(sizeof(queue));
    result->max_size = max_size;
    result->size = 0;
    result->head = NULL;
    result->tail = NULL;
    pthread_cond_init(&(result->cv), NULL);
    pthread_mutex_init(&(result->m), NULL);
    return result;
}

void queue_destroy(queue *this) {
    /* Your code here */
    queue_node* current = this->head;
    queue_node* temp;
    while(current != NULL){
        temp = current;
        current = current->next;
        free(temp);
    }
    pthread_mutex_destroy(&(this->m));
    pthread_cond_destroy(&(this->cv));
    free(this);
    this = NULL;
}

void queue_push(queue *this, void *data) {
    /* Your code here */
    queue_node* new = (queue_node*)malloc(sizeof(queue_node));
    new->data = data;
    new->next = NULL;

    pthread_mutex_lock(&this->m);
    //check if queue is bounded before blocking
    if(this->max_size > 0){
        while(this->size == this->max_size){
            pthread_cond_wait(&(this->cv), &(this->m));
        }
    }

    if(this->tail != NULL){
        this->tail->next = new;
    }
    else{
        this->head = new;
    }
    this->tail = new;
    this->size++;
    if(this->size == 1){
        pthread_cond_broadcast(&this->cv);
    }
    pthread_mutex_unlock(&this->m);
}

void *queue_pull(queue *this) {
    /* Your code here */
    pthread_mutex_lock(&this->m);
    while(this->size == 0){
        pthread_cond_wait(&(this->cv), &(this->m));
    }
    void* result = this->head->data;
    queue_node* temp = this->head->next;
    free(this->head);
    this->head = temp;
    if(this->size == 1){
        this->tail = NULL;
    }
    this->size--;

    if(this->size + 1 == this->max_size){
        pthread_cond_broadcast(&this->cv);
    }
    pthread_mutex_unlock(&this->m);    
    return result;
}
