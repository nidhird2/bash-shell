/**
 * Extreme Edge Cases
 * CS 241 - Spring 2020
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

int checkIfMatch(char** actual, char** expected){
    printf("Here2 \n");
    if(sizeof(actual) != sizeof(expected)){
	return 0;
    }
    int len = sizeof(actual);
    int i = 0;
    for(i = 0; i < len - 1; i++){
	if(strcmp(actual[i], expected[i]) != 0){
		return 0;
	}
    }
    printf("Here3 \n");
    return 1;
}

int checkNull(char **(*camelCaser)(const char *), void (*destroy)(char**)){
    char* input = NULL;
    char** output = camelCaser(input);
    int result = 1;
    if(output != NULL){
	result = 0;
    }
    destroy(output);
    return result;
}

int checkOneSentence(char **(*camelCaser)(const char*), void (*destroy)(char**)){
    char* input = "hello world.";
    char* exp[] = {"helloWord", NULL};
    char **output = camelCaser(input);
    int result = 1;
    if(checkIfMatch(output, exp)){
	result = 0;
    }
    destroy(output);
    return result;
}
    

int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
   int result = 1;
   if(!checkNull(camelCaser, destroy)){
	printf("checkNull failed.\n");
	return 0;
   }
   if(!checkOneSentence(camelCaser, destroy)){
	printf("check one sentence failed.\n");
	result = 0;
   }
   return result;
}
