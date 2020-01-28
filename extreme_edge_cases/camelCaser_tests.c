/**
 * Extreme Edge Cases
 * CS 241 - Spring 2020
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

int findSize(char** actual){
    int count = 0;
    int idx = 0;
    while(actual[idx] != NULL){
	count++;
	idx++;
    }
    return count;
}

int checkIfMatch(char** actual, char** expected){
    int len = findSize(actual);
    int len2 = findSize(expected);
    if(len != len2){
	return 0;
    }
    int i = 0;
//    printf("len: %d\n", len);
    for(i = 0; i < len; i++){
	//printf("ac: %s\n", actual[i]);
	//printf("ex: %s\n", expected[i]);
	if(strcmp(actual[i], expected[i]) != 0){
		return 0;
	}
    }
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
    char* exp[] = {"helloWorld", NULL};
    char **output = camelCaser(input);
    int result = 1;
    if(!checkIfMatch(output, exp)){
	result = 0;
    }
    destroy(output);
    return result;
}

int check3(char**(*camelCaser)(const char*), void (*destroy)(char**)){
    char* input = "faslfjsljlkxn,mcngsaAGSSDF";
    char* exp[] ={"faslfjsljlkxn", NULL};
    char **output = camelCaser(input);
    int result = 1;
    if(!checkIfMatch(output, exp)){
	result = 0;
    }
    destroy(output);
    return result;
}

int check4(char**(*camelCaser)(const char*), void(*destroy)(char**)){
    char* input = "  spaceSpace space     . Space space.";
    char* exp[] = {"spacespaceSpace","spaceSpace",NULL};
    char **output = camelCaser(input);
    int result = 1;
    if(!checkIfMatch(output, exp)){
	result = 0;
    }
    destroy(output);
    return result;
    }

int check5(char**(*camelCaser)(const char*), void(*destroy)(char**)){
    char* input = "";
    char* exp[] = {NULL};
    char **output = camelCaser(input);
    int result = 1;
    if(!checkIfMatch(output, exp)){
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
	return 0;
   }

   if(!check3(camelCaser,destroy)){
	printf("check3: no sentences failed.\n");
	return 0;
   }
  
   if(!check4(camelCaser, destroy)){
	printf("check4: failed\n");
	return 0;
   }

   if(!check5(camelCaser, destroy)){
	printf("empty string input failed\n");
	return 0;
   }
   return result;
}
