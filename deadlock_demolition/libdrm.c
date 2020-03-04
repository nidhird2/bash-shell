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

struct drm_t {
    // Declare your struct's variables here. Think about what you will need.
    // Hint: You will need at least a synchronization primitive.
};

drm_t *drm_init() {
    /* Your code here */
    return NULL;
}

int drm_post(drm_t *drm, pthread_t *thread_id) {
    /* Your code here */
    return 0;
}

int drm_wait(drm_t *drm, pthread_t *thread_id) {
    /* Your code here */
    return 0;
}

void drm_destroy(drm_t *drm) {
    /* Your code here */
    return;
}
