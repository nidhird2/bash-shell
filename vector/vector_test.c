/**
 * Vector
 * CS 241 - Spring 2020
 */
#include "vector.h"
#include <stdio.h>
int main(int argc, char *argv[]) {
    // Write your test cases here
    vector* tester = int_vector_create();
    unsigned int* nums[5];
    for(unsigned int i = 0; i < 5; i++){
        //nums[i] = (int*) malloc(sizeof(int*));
        nums[i] = &i;
        vector_push_back(tester, (void*)nums[i]);
        printf("%d : %d\n", i, *((int*)vector_get(tester, i)));
    }
    vector_destroy(tester);
    //return 0;
}
