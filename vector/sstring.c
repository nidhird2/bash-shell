/**
 * Vector
 * CS 241 - Spring 2020
 */
#include "sstring.h"
#include "vector.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <string.h>

int findTarget(const char*, size_t, size_t, const char*);

struct sstring {
    vector* v;
};

sstring *cstr_to_sstring(const char *input) {
    sstring* result = (struct sstring*)malloc(sizeof(struct sstring));
    result->v = char_vector_create();
    const char* current = input;
    while(*current){
        vector_push_back(result->v, (void*)current);
        current++;
    }
    return result;
}

char *sstring_to_cstr(sstring *input) {
    size_t size = vector_size(input->v);
    char* result = (char*)malloc(sizeof(char) * (size + 1));
    for(size_t i = 0; i < size; i++){
        result[i] = *(char*)vector_get(input->v, i);
    }
    result[size] = '\0';
    return result;
}

int sstring_append(sstring *this, sstring *addition) {
    
    if(this == NULL || addition == NULL){
        return -1;
    }
    size_t b = vector_size(addition->v);
    for(size_t i = 0; i < b; i++){
        vector_push_back(this->v, (void*)vector_get(addition->v, i));
    }
    return vector_size(this->v);
}

vector *sstring_split(sstring *this, char delimiter) {
    // your code goes here
    vector* result = string_vector_create();
    if(this == NULL){
        return result;
    }
    size_t i = 0;
    size_t o = vector_size(this->v);
    for(size_t j = 0; j < o; j++){
        char* current = vector_get(this->v, j);
        
        if(*current == delimiter){
            int count = j-i + 1;
            char* temp = (char*)malloc(sizeof(char) * count);
            for(size_t k = i; k < j; k++){
                temp[k-i] = *(char*)vector_get(this->v, k);
            }
            temp[count - 1] = '\0';
            vector_push_back(result, temp);
            free(temp);
            i = j + 1;
        }
        else{
            continue;
        }
    }
    int count = o-i + 1;
    char* temp = (char*)malloc(sizeof(char) * count);
    for(size_t k = i; k < o; k++){
        temp[k-i] = *(char*)vector_get(this->v, k);
    }
    temp[count - 1] = '\0';
    vector_push_back(result, temp);
    free(temp);
    return result;
}

int sstring_substitute(sstring *this, size_t offset, char *target,
                       char *substitution) {
    // your code goes here
    size_t temp_size = vector_size(this->v);
    assert(offset < temp_size);
    assert(offset >= 0);
    char* temp = sstring_to_cstr(this);
    int res = findTarget(temp, temp_size, offset, target);
    if(res == -1){
        free(temp);
        return -1;
    }
    int target_len =strlen(target);
    int sub_len =strlen(substitution);
    vector_clear(this->v);
    for(int i = 0; i < res; i++){
        vector_push_back(this->v, &temp[i]);
    }
    for(int i = 0; i < sub_len; i++){
        vector_push_back(this->v, &substitution[i]);
    }
    for(int i = res + target_len; i < (int)temp_size; i++){
        vector_push_back(this->v, &temp[i]);
    }
    free(temp);
    return 0;
}
int findTarget(const char* og, size_t og_length, size_t offset, const char*target){
    const char* current = og;
    const char* current2 = target;
        for(size_t i = offset; i < og_length; i++){
            //printf("i: %lu\n", i);
            current = og + i;
            current2 = target;
            while(*current == *current2){
                //printf("current: %c current2: %c\n", *current, *current2);
                current++;
                current2++;
                if(*current2 == '\0' || *current == '\0'){
                    break;
                }
            }
        if(*current2 == '\0'){
            //printf("current2 is null, i: %lu\n", i);
            return i;
        }
        if(*current == '\0'){
            //printf("current is null, i: %lu\n", i);
            return -1;
        }
    }
    return -1; 
}

char *sstring_slice(sstring *this, int start, int end) {
    // your code goes here
    assert(this);
    assert(end>=start);
    size_t len = (end-start) + 1;
    char* result = (char*)malloc(sizeof(char) * len);
    for(int i = start; i < end; i++){
        result[i-start] = *(char*)vector_get(this->v, i);
    }
    result[len-1] = '\0';
    return result;
}

void sstring_destroy(sstring *this) {
    vector_destroy(this->v);
    free(this);
    this = NULL;
}
