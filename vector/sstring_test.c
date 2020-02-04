/**
 * Vector
 * CS 241 - Spring 2020
 */
#include "sstring.h"
#include <stdio.h>

void print_vector(vector* v){
    size_t s = vector_size(v);
    for(size_t i = 0; i < s; i++){
        printf("%lu: %s\n", i, vector_get(v, i));
    }
    printf("\n");
}


int main(int argc, char *argv[]) {
    //char* a = "hello ";
    //sstring* temp = cstr_to_sstring(a);
    //char* bb = "!Oranges, Apples, and Grapes!";
    //sstring* temp2 = cstr_to_sstring(bb);
    return 0;
}
