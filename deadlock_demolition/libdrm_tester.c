/**
 * Deadlock Demolition
 * CS 241 - Spring 2020
 */
#include "libdrm.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static drm_t *drm;
static drm_t *drm2;
static int count = 0;

void *f(void *id) {
    int *result = malloc(sizeof(int));
    drm_wait(drm, id);

    if (count == 0)
        printf("Thread %zu was first\n", *((pthread_t *)id));
    else
        printf("Thread %zu was second\n", *((pthread_t *)id));

    count++;
    *result = count;
    drm_post(drm, id);

    return result;
}

void *f1(void *id) {
    int *result = malloc(sizeof(int));
    *result = count;
    if(drm_wait(drm, id)){
        sleep(1);
        if(drm_wait(drm2, id)){
            printf("count: %d\n", count);
            count++;
            *result = count;
        }
        drm_post(drm2, id);
    }
    drm_post(drm, id);

    // if (count == 0)
    //     printf("Thread %zu was first\n", *((pthread_t *)id));
    // else
    //     printf("Thread %zu was second\n", *((pthread_t *)id));
    return result;
}

void *f2(void *id) {
    int *result = malloc(sizeof(int));
    *result = count;
    if(drm_wait(drm2, id)){
        sleep(1);
        if(drm_wait(drm, id)){
            printf("count: %d\n", count);
            count++;
            *result = count;
        }
        drm_post(drm, id);
    }
    drm_post(drm2, id);

    // if (count == 0)
    //     printf("Thread %zu was first\n", *((pthread_t *)id));
    // else
    //     printf("Thread %zu was second\n", *((pthread_t *)id));
    return result;
}

int testMainOG(){
    // Example usage of DRM
    drm = drm_init();
    pthread_t t1, t2;
    pthread_create(&t1, NULL, f, &t1);
    pthread_create(&t2, NULL, f, &t2);
    int *r1, *r2;
    pthread_join(t1, (void **)&r1);
    pthread_join(t2, (void **)&r2);
    printf("Thread 1 with ID %zu returned %d\n", t1, *r1);
    printf("Thread 2 with ID %zu returned %d\n", t2, *r2);
    free(r1);
    free(r2);
    drm_destroy(drm);
    return 0;
}

int testTwo(){
    drm = drm_init();
    drm2 = drm_init();
     pthread_t t1, t2;
    pthread_create(&t1, NULL, f2, &t1);
    pthread_create(&t2, NULL, f1, &t2);
    int *r1, *r2;
    pthread_join(t2, (void **)&r2);
    pthread_join(t1, (void **)&r1);
    printf("Thread 1 with ID %zu returned %d\n", t1, *r1);
    printf("Thread 2 with ID %zu returned %d\n", t2, *r2);
    free(r1);
    free(r2);
    drm_destroy(drm);
    drm_destroy(drm2);
    return 0;
}


int main() {
    return testTwo();
}
