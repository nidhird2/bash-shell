/**
 * Deadlock Demolition
 * CS 241 - Spring 2020
 */
#ifndef __LIBDRS_H__
#define __LIBDRS_H__

#include <pthread.h>

typedef struct drm_t drm_t;

/**
 * Initialize a deadlock-resistant mutex (drm). The new lock should be added to
 * a Resource Allocation Graph.
 *
 * Attempting to initialize a drm that has already been initialized is undefined
 * behavior.
 *
 * @return A pointer to the initialized drm.
 */
drm_t *drm_init();

/**
 * Unlocks the given drm. Remove the appropriate edge from your Resource
 * Allocation Graph.
 *
 * @param drm - The drm to be unlocked.
 * @param thread_id - The ID of the thread that is unlocking the drm.
 * @return :
 *    0 if the specified thread is not able to unlock the given drm.
 *    1 if the specified thread is able to unlock the given drm.
 */
int drm_post(drm_t *drm, pthread_t *thread_id);

/**
 * Attempts to lock the given drm with the specified thread. To make this
 * attempt, create the appropriate edge in your Resource Allocation Graph and
 * check for a cycle. If no cycle exists, allow the thread to wait on the drm.
 * Otherwise, remove the edge from your Resource Allocation Graph, and return.
 *
 * @param drm - The drm to be locked.
 * @param thread_id - The ID of the thread that is locking the drm.
 * @return :
 *    0 if attempting to lock this drm with the specified thread would cause
 *      deadlock.
 *    1 if the drm is locked by the specified thread.
 */
int drm_wait(drm_t *drm, pthread_t *thread_id);

/**
 * Destroys the drm, as well as its allocated resources.
 * Remove the vertex representing this drm from the Resource Allocation Graph.
 *
 * @param drm - The drm to be destroyed.
 */
void drm_destroy(drm_t *drm);

#endif
