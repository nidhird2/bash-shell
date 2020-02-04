/**
 * Vector
 * CS 241 - Spring 2020
 */
#include "vector.h"
#include <stdio.h>
int main(int argc, char *argv[]) {
    vector* tester = int_vector_create();
    for(unsigned int i = 0; i < 5; i++){
        vector_push_back(tester, (void*)&i);
        printf("%d:%d  ", i, *((int*)vector_get(tester, i)));
    }
    printf("\n");
    printf("vector size : %lu\n", vector_size(tester));
    printf("vector capacity :%lu\n  ", vector_capacity(tester));
    vector_resize(tester, 15);
    printf("vector size : %lu\n", vector_size(tester));
    printf("vector capacity :%lu\n  ", vector_capacity(tester));
    for(unsigned int i = 0; i < 15; i++){
        printf("%d:%d  ", i, *((int*)vector_get(tester, i)));
    }
    printf("\n");
    printf("erasing idxs 0, 2, 4\n");
    vector_erase(tester, 4);
    vector_erase(tester, 2);
    vector_erase(tester, 0);
    printf("vector size : %lu\n", vector_size(tester));
    printf("vector capacity :%lu\n  ", vector_capacity(tester));
    for(unsigned int i = 0; i < 12; i++){
        printf("%d:%d  ", i, *((int*)vector_get(tester, i)));
    }
    printf("\nvector inserting\n");
    int a = 2;
    int b = 12;
    int c = 122;
    vector_insert(tester, 12, &a);
    vector_insert(tester, 13, &b);
    vector_insert(tester, 14, &c);
    printf("vector size : %lu\n", vector_size(tester));
    printf("vector capacity :%lu\n  ", vector_capacity(tester));
    for(unsigned int i = 0; i < 15; i++){
        printf("%d:%d  ", i, *((int*)vector_get(tester, i)));
    }
    printf("\n");
    vector_destroy(tester);
    return 0;
}
