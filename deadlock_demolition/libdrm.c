/**
 * Deadlock Demolition
 * CS 241 - Spring 2020
 */
#include "graph.h"
#include "libdrm.h"
#include "set.h"
#include <pthread.h>

// You probably will need some global variables here to keep track of the
// resource allocation graph.
graph* g;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

struct drm_t {
    // Declare your struct's variables here. Think about what you will need.
    // Hint: You will need at least a synchronization primitive.
    pthread_mutex_t m;
};


int check_if_circular(void){
    return 0;
}

int check_if_edge_exists(void* source, void* dest){
    if(graph_adjacent(g, source, dest)){
        return 1;
    }
    return 0;
}

drm_t *drm_init() {
    /* Your code here */
    pthread_mutex_lock(&m);
    if(g == NULL){
        g = shallow_graph_create();
    }
    drm_t* result = malloc(sizeof(drm_t));
    pthread_mutex_init(&result->m, NULL);
    graph_add_vertex(g, result);
    pthread_mutex_unlock(&m);
    return result;
}

int drm_post(drm_t *drm, pthread_t *thread_id) {
    pthread_mutex_lock(&m);
    /* Your code here */
    if(!graph_contains_vertex(g, thread_id)){
        pthread_mutex_unlock(&m);
        return 1;
    }
    if(!check_if_edge_exists(drm, thread_id)){
        pthread_mutex_unlock(&m);
        return 1;
    }
    graph_remove_edge(g, drm, thread_id);
    pthread_mutex_unlock(&drm->m);
    pthread_mutex_unlock(&m);
    return 0;
}

int drm_wait(drm_t *drm, pthread_t *thread_id) {
    pthread_mutex_lock(&m);
    /* Your code here */
    //check if thread exists in graph
    if(!graph_contains_vertex(g, thread_id)){
        graph_add_vertex(g, thread_id);
    }
    //check if thread owns the mutex already and return early
    if(check_if_edge_exists(drm, thread_id)){
        pthread_mutex_unlock(&m);
        return 1;
    }

    //add edge to graph
    graph_add_edge(g, thread_id, drm);
    //check circular wait
    if(check_if_circular()){
        graph_remove_edge(g, thread_id, drm);
        pthread_mutex_unlock(&m);
        return 1;
    }

    //lock if no circular wait
    pthread_mutex_lock(&drm->m);
    
    //change graph 
    graph_remove_edge(g, thread_id, drm);
    graph_add_edge(g, drm, thread_id);
    pthread_mutex_unlock(&m);
    return 0;
}

void drm_destroy(drm_t *drm) {
    /* Your code here */
    pthread_mutex_destroy(&drm->m);
    free(drm);
    //remove from graph
    pthread_mutex_lock(&m);
    graph_remove_vertex(g, drm);
    pthread_mutex_unlock(&m);
    return;
}
