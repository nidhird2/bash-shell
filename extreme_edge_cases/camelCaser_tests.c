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
//    printf("act len1: %d\n", len);
//    printf("exp len2: %d\n", len2);
    if(len != len2){
	return 0;
    }
    int i = 0;
//    printf("len: %d\n", len);
    for(i = 0; i < len; i++){
//	printf("ac: %s\n", actual[i]);
//	printf("ex: %s\n", expected[i]);
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

int noSpaces(char**(*camelCaser)(const char*), void (*destroy)(char**)){
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

int whitespace(char**(*camelCaser)(const char*), void(*destroy)(char**)){
    char* input = "  spaceSpace space  !   	 Space space.";
    char* exp[] = {"spacespaceSpace","spaceSpace",NULL};
    char **output = camelCaser(input);
    int result = 1;
    if(!checkIfMatch(output, exp)){
	result = 0;
    }
    destroy(output);
    return result;
}

int checkEmpty(char**(*camelCaser)(const char*), void(*destroy)(char**)){
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

int checkAllSpaces(char **(*camelCaser)(const char*), void(*destroy)(char**)){
    char* input = "	   	";
    char* exp[] = {NULL};
    char **output = camelCaser(input);
    int result = 1;
    if(!checkIfMatch(output, exp)){
	result = 0;
    }
    destroy(output);
    return result;
}

int checkAllNums(char **(*camelCaser)(const char*), void(* destroy)(char**)){
    char* input = " 1 2 3 	. 4567 89. 012.  345 678.";
    char* exp[] = {"123", "456789", "012", "345678", NULL};
    char **output = camelCaser(input);
    int result = 1;
    if(!checkIfMatch(output,exp)){
	result = 0;
    }
    destroy(output);
    return result;
}

int checkMultPunc(char **(*camelCaser)(const char*), void(* destroy)(char**)){
    char* input = "  abc dEF  . ! ? gHiJ\",. KLMNO..";
    char* exp[] = {"abcDef","","","ghij","","","klmno","",NULL};
    char **output = camelCaser(input);
    int result = 1;
    if(!checkIfMatch(output,exp)){
	result = 0;
    }
    destroy(output);
    return result;
}

int checkOtherChars(char **(*camelCaser)(const char*), void(*destroy)(char**)){
    char* input = "@s#o &*abs  \n";
    char* exp[] = {"","s","o","",NULL};
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
	//printf("checkNull failed.\n");
	return 0;
   }
   if(!checkOneSentence(camelCaser, destroy)){
	//printf("check one sentence failed.\n");
	return 0;
   }

   if(!noSpaces(camelCaser,destroy)){
	//printf("noSpaces: no sentences failed.\n");
	return 0;
   }
  
   if(!whitespace(camelCaser, destroy)){
	//printf("check4:whitespace failed\n");
	return 0;
   }

   if(!checkEmpty(camelCaser, destroy)){
	//printf("empty string input failed\n");
	return 0;
   }

   if(!checkAllSpaces(camelCaser,destroy)){
	//printf("all spaces input failed\n");
	return 0;
   }

   if(!checkAllNums(camelCaser, destroy)){
	//printf("check all nums input failed\n");
	return 0;
   }

   if(!checkMultPunc(camelCaser, destroy)){
	//printf("multpunc input failed\n");
	return 0;
   }
   
   if(!checkOtherChars(camelCaser, destroy)){
	//printf("other chars input failed\n");
	return 0;
   }
   
   return result;
}
