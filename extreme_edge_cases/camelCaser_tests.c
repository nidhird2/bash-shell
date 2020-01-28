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
    if(sizeof(actual) != sizeof(expected)){
	return 0;
    }
    int len = sizeof(actual) / sizeof(actual[0]);
    int i = 0;
    for(i = 0; i < (len - 1); i++){
	printf("YIKES\n");
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
    char* exp[] = {"helloWord", NULL};
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
   return result;
}
