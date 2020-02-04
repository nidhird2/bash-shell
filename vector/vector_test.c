/**
 * Vector
 * CS 241 - Spring 2020
 */
#include "vector.h"
#include <stdio.h>

void print_vector(vector* v){
    size_t s = vector_size(v);
    printf("size: %lu\n", vector_size(v));
    printf("capacity: %lu\n", vector_capacity(v));
    for(size_t i = 0; i < s; i++){
        printf("%lu:%d ", i, *(int*)vector_get(v, i));
    }
    printf("\n");
}

int test_vector_set(){
    vector* tester = int_vector_create();
    for(int i = 0; i < 10; i++){
        vector_push_back(tester, &i);
    }
    int a = 15;
    vector_set(tester, 2, &a);
    a = 100;
    vector_set(tester, 0, &a);
    a = 32;
    vector_set(tester, 9, &a);
    if(*(int*)vector_get(tester, 2) != 15){
        vector_destroy(tester);
        return 0;
    }
    else if(*(int*)vector_get(tester, 0) != 100){
        vector_destroy(tester);
        return 0;
    }
    else if(*(int*)vector_get(tester, 9) != 32){
        vector_destroy(tester);
        return 0;
    }
    vector_destroy(tester);
    return 1;
}

int test_vector_pop_back(){
    vector* tester = int_vector_create();
    for(int i = 0; i < 10; i++){
        vector_push_back(tester, &i);
    }
    for(int i = 0; i < 10; i++){
        vector_pop_back(tester);
    }
    vector_destroy(tester);
    return 1;
}


int test_vector_resize(){
    vector* tester = int_vector_create();
    for(int i = 0; i < 15; i++){
        vector_push_back(tester, &i);
    }
    size_t cap_before = vector_capacity(tester);
    vector_resize(tester, 5);
    if(vector_size(tester) != 5){
        vector_destroy(tester);
        return 0;
    } else if(vector_capacity(tester) != cap_before){
        vector_destroy(tester);
        return 0;
    }
    vector_resize(tester, 20);
    if(vector_size(tester) != 20){
        vector_destroy(tester);
        return 0;
    } else if(vector_capacity(tester) != 32){
        vector_destroy(tester);
        return 0;
    } else if(0 != *(int*)vector_get(tester, 19)){
        vector_destroy(tester);;
        return 0;
    }
    else if(0 != *(int*)vector_get(tester, 8)){
        vector_destroy(tester);;
        return 0;
    }
    vector_destroy(tester);
    return 1;
}

int test_vector_reserve(){
    vector* tester = int_vector_create();
    for(int i = 0; i < 10; i++){
        vector_push_back(tester, &i);
    }
    vector_reserve(tester, 50);
    int a = 13;
    vector_push_back(tester, &a);
    if(vector_capacity(tester) != 64){
        vector_destroy(tester);
        return 0;
    }
    vector_reserve(tester, 0);
    if(vector_capacity(tester) != 64){
        vector_destroy(tester);
        return 0;
    }
    vector_reserve(tester, 20);
    if(vector_capacity(tester) != 64){
        vector_destroy(tester);
        return 0;
    }
    vector_destroy(tester);
    return 1;
}

int test_vector_erase(){
    vector* tester = int_vector_create();
    for(int i = 0; i < 10; i++){
        vector_push_back(tester, &i);
    }
    vector_erase(tester, 0);
    for(int i = 0; i < 9; i++){
        if(*(int*)vector_get(tester, (size_t)i) != i + 1){
            vector_destroy(tester);
            return 0;
        }
    }
    vector_erase(tester, 4);
    for(int i = 4; i < 8; i++){
        if(*(int*)vector_get(tester, (size_t)i) != i + 2){
            vector_destroy(tester);
            return 0;
        }
    }
    vector_erase(tester, 7);
    if(*(int*)vector_get(tester, (size_t)6) != 8){
        vector_destroy(tester);
        return 0;
    }
    if(vector_size(tester) != 7){
        vector_destroy(tester);
        return 0;
    }
    vector_destroy(tester);
    return 1;
}

int test_vector_insert(){ 
    vector* tester = int_vector_create();
    for(int i = 0; i < 9; i++){
        vector_insert(tester, (size_t)i, &i);
    }
    int i = 100;
    vector_insert(tester, 0, &i);
    if(*(int*)vector_get(tester, 0) != 100){
        vector_destroy(tester);
        return 0;
    }
    for(int i = 1; i < 10; i++){
        if(*(int*)vector_get(tester, (size_t)i) != i - 1){
            vector_destroy(tester);
            return 0;
        }
    }
    i = 54;
    vector_insert(tester, 3, &i);
    if(*(int*)vector_get(tester, 3) != 54){
        vector_destroy(tester);
        return 0;
    }
    for(int i = 4; i < 11; i++){
        if(*(int*)vector_get(tester, (size_t)i) != i - 2){
            vector_destroy(tester);
            return 0;
        }
    }
    vector_destroy(tester);
    return 1;
}

int test_many_insert(){
    vector* tester = int_vector_create();
    for(int i = 0; i < 10; i++){
        vector_insert(tester, i, &i);
    }
    print_vector(tester);
    for(int i = 100; i < 109; i++){
        vector_insert(tester, 4, &i);
        print_vector(tester);
    }
    vector_destroy(tester);
    return 1;
}

int main(int argc, char *argv[]) {
    // if(!test_vector_set()){
    //     printf("vector set test failed\n");
    //     return 0;
    // } else if(!test_vector_pop_back()){
    //     printf("vector pop back test failed\n");
    //     return 0;
    // }
    // if(!test_vector_resize()){
    //     printf("vector resize test failed\n");
    //     return 0;
    // }
    //  if(!test_vector_reserve()){
    //     printf("vector reserve test failed\n");
    //     return 0;
    // }
    // else if(!test_vector_erase()){
    //     printf("vector erase test failed\n");
    //     return 0;
    //} 
    if(!test_vector_insert()){
        printf("vector insert test failed\n");
        return 0;
    }
    if(!test_many_insert()){
        printf("test insert many failed\n");
        return 0;
    }
    printf("SUCCESS\n");
    return 0;
    // for(unsigned int i = 0; i < 5; i++){
    //     vector_push_back(tester, (void*)&i);
    //     printf("%d:%d  ", i, *((int*)vector_get(tester, i)));
    // }
    // printf("\n");
    // printf("vector size : %lu\n", vector_size(tester));
    // printf("vector capacity :%lu\n  ", vector_capacity(tester));
    // vector_resize(tester, 15);
    // printf("vector size : %lu\n", vector_size(tester));
    // printf("vector capacity :%lu\n  ", vector_capacity(tester));
    // for(unsigned int i = 0; i < 15; i++){
    //     printf("%d:%d  ", i, *((int*)vector_get(tester, i)));
    // }
    // printf("\n");
    // printf("erasing idxs 0, 2, 4\n");
    // vector_erase(tester, 4);
    // vector_erase(tester, 2);
    // vector_erase(tester, 0);
    // printf("vector size : %lu\n", vector_size(tester));
    // printf("vector capacity :%lu\n  ", vector_capacity(tester));
    // for(unsigned int i = 0; i < 12; i++){
    //     printf("%d:%d  ", i, *((int*)vector_get(tester, i)));
    // }
    // printf("\nvector inserting\n");
    // int a = 2;
    // int b = 12;
    // int c = 122;
    // vector_insert(tester, 12, &a);
    // vector_insert(tester, 13, &b);
    // vector_insert(tester, 14, &c);
    // printf("vector size : %lu\n", vector_size(tester));
    // printf("vector capacity :%lu\n  ", vector_capacity(tester));
    // for(unsigned int i = 0; i < 15; i++){
    //     printf("%d:%d  ", i, *((int*)vector_get(tester, i)));
    // }

    // printf("\n");
    // vector_destroy(tester);
    //return 0;
}
