/**
 * Deadlock Demolition
 * CS 241 - Spring 2020
 */
#include "graph.h"
#include "libdrm.h"
#include "set.h"
#include "queue.h"
#include <stdio.h>
#include <pthread.h>

// You probably will need some global variables here to keep track of the
// resource allocation graph.
graph* g;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

struct drm_t {
    // Declare your struct's variables here. Think about what you will need.
    // Hint: You will need at least a synchronization primitive.
    pthread_mutex_t m;
};

//O is FALSE
//1 IS TRUE

int recCyclic(void* v, dictionary* visited, dictionary* recStack){
    //printf("current: %p\n", vertex);
    int f = 0;
    int t = 1;
    if(*(int*)dictionary_get(visited, v) == false){
        //update rec status
        dictionary_set(visited, v, &t);
        dictionary_set(recStack, v, &t);

        //check adj neighbors
        vector* neigh = graph_neighbors(g, v);
        size_t neigh_length = vector_size(neigh);
        for(size_t i = 0; i < neigh_length; i++){
            void* current = vector_get(neigh, i);
            int if_visited = *(int*)dictionary_get(visited, current);
            int if_rec = *(int*)dictionary_get(recStack, current);

            if(if_visited == false && recCyclic(current, visited, recStack)){
                vector_destroy(neigh);
                return true;
            }
            else if(if_rec == true){
                vector_destroy(neigh);
                return true;
            }
        }
        vector_destroy(neigh);
    }
    dictionary_set(recStack, v, &f);
    return false;
}


int check_if_circular(void){
    vector * vertexes = graph_vertices(g);
    size_t num_verticies = vector_size(vertexes);
    //cycle detection algo from: 
    //https://www.geeksforgeeks.org/detect-cycle-in-a-directed-graph-using-bfs/
    dictionary* visited = shallow_to_int_dictionary_create();
    dictionary* recStack = shallow_to_int_dictionary_create();
    int f = 0;
    //printf("num vertexes: %lu\n", num_verticies);
    for(size_t i = 0; i < num_verticies; i++){
        void* current_vertex = vector_get(vertexes, i);
        //printf("current vertex: %p\n", current_vertex);
        dictionary_set(visited, current_vertex, &f);
        dictionary_set(recStack, current_vertex, &f);
        //mint* val = (int*)dictionary_get(recStack, current_vertex);
        //printf("val* is: %p\n", val);
        //printf("val is: %d\n", *val);
    }
    for(size_t i = 0; i < num_verticies; i++){
        void* current_vertex = vector_get(vertexes, i);
        if(recCyclic(current_vertex, visited, recStack)){
            dictionary_destroy(visited);
            dictionary_destroy(recStack);
            vector_destroy(vertexes);
            return true;
        }
    }
    dictionary_destroy(visited);
    dictionary_destroy(recStack);
    vector_destroy(vertexes);
    return false;
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
    printf("drm vtx: %p\n", result);
    pthread_mutex_init(&result->m, NULL);
    graph_add_vertex(g, result);
    pthread_mutex_unlock(&m);
    return result;
}

int drm_post(drm_t *drm, pthread_t *thread_id) {
    pthread_mutex_lock(&m);
    printf("unlock request- drm: %p thread: %p\n", drm, thread_id);
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
    if(graph_vertex_antidegree(g, drm) >= 1){
        pthread_cond_broadcast(&cv);
    }
    pthread_mutex_unlock(&m);
    return 0;
}

int drm_wait(drm_t *drm, pthread_t *thread_id) {
    pthread_mutex_lock(&m);
    printf("lock request- drm: %p thread: %p\n", drm, thread_id);
    /* Your code here */
    //check if thread exists in graph
    if(!graph_contains_vertex(g, thread_id)){
        printf("thread vtx: %p\n", thread_id);
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
        printf("circular graph!\n");
        graph_remove_edge(g, thread_id, drm);
        pthread_mutex_unlock(&m);
        return 1;
    }
    printf("no circular wait!\n");
    //lock if no circular wait
    //printf("about to lock!\n");
    pthread_mutex_unlock(&m);
    while(graph_vertex_degree(g, drm) >= 1){
        pthread_cond_wait(&cv, &m);
    }
    printf("about to lock!\n");
    pthread_mutex_lock(&drm->m);
    //change graph 
    graph_remove_edge(g, thread_id, drm);
    graph_add_edge(g, drm, thread_id);
    pthread_mutex_unlock(&m);
    return 0;
}

void drm_destroy(drm_t *drm) {
    /* Your code here */
    pthread_mutex_lock(&m);
    pthread_mutex_destroy(&drm->m);
    free(drm);
    //remove from graph
    graph_remove_vertex(g, drm);
    pthread_mutex_unlock(&m);
    return;
}
