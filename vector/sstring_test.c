/**
 * Vector
 * CS 241 - Spring 2020
 */
#include "sstring.h"
#include <stdio.h>
#include <string.h>

void print_vector(vector* v){
    size_t s = vector_size(v);
    for(size_t i = 0; i < s; i++){
        printf("%lu: %s\n", i, vector_get(v, i));
    }
    printf("\n");
}
int test_substitution(){
    sstring *replace_me = cstr_to_sstring("This is a {} day, {}!!");
    int res = sstring_substitute(replace_me, 18, "{}!", "friend");
    char* res1 = sstring_to_cstr(replace_me); // == "This is a {} day, friend!"
    if(strcmp(res1, "This is a {} day, friend!") != 0){
        printf("res: %d res1: %s\n", res, res1);
        free(res1);
        sstring_destroy(replace_me);
        return 0;
    }
    res = sstring_substitute(replace_me, 0, "{}", "good");
    char* res2 = sstring_to_cstr(replace_me);
    if(strcmp(res2, "This is a good day, friend!") != 0){
        printf("res: %d res1: %s\n", res, res1);
        free(res1);
        free(res2);
        sstring_destroy(replace_me);
        return 0;
    }
    free(res1);
    free(res2);
    sstring_destroy(replace_me);
    return 1;
}

int test_slice(){
    sstring *slice_me = cstr_to_sstring("1234567890");
    char* res = sstring_slice(slice_me, 2, 5);
    int temp = strcmp(res, "345");
    sstring_destroy(slice_me);
    free(res);
    if(temp != 0){
        return 0;
    }
    return 1;
}


int main(int argc, char *argv[]) {
    //char* a = "hello ";
    //sstring* temp = cstr_to_sstring(a);
    //char* bb = "!Oranges, Apples, and Grapes!";
    //sstring* temp2 = cstr_to_sstring(bb);
    if(!test_substitution()){
        printf("failed substitution.\n");
        return 0;
    }
    if(!test_slice()){
        printf("failed slice\n");
        return 0;
    }
    printf("sstring tests passed\n");
    return 0;
}
