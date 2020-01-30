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
    int idx = 0;
    while(actual[idx] != NULL){
	idx++;
    }
    return idx;
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

int testNoPunc(char **(*camelCaser)(const char*), void(*destroy)(char **)){
    char* input = " dfk   lajknkb  cvxnlv    ksdnkla ";
    char* exp[] = {NULL};
    char** output = camelCaser(input);
    int result = checkIfMatch(output,exp);
    destroy(output);
    return result;
}

int checkEmpty(char**(*camelCaser)(const char*), void(*destroy)(char**)){
    char* input = "";
    char* exp[] = {NULL};
    char **output = camelCaser(input);
    int result = checkIfMatch(output, exp);
    destroy(output);
    return result;
}

int checkAllSpaces(char **(*camelCaser)(const char*), void(*destroy)(char**)){
    char* input = "	   	";
    char* exp[] = {NULL};
    char **output = camelCaser(input);
    int result = checkIfMatch(output, exp);
    destroy(output);
    return result;
}


int checkOneSentence(char **(*camelCaser)(const char*), void (*destroy)(char**)){
    char* input = "HELLO WORLD.";
    char* exp[] = {"helloWorld", NULL};
    char **output = camelCaser(input);
    int result = checkIfMatch(output, exp);
    destroy(output);
    return result;
}

int noSpaces(char**(*camelCaser)(const char*), void (*destroy)(char**)){
    char* input = "faslfjsljlkxn,mcngsaAGSSDF";
    char* exp[] ={"faslfjsljlkxn", NULL};
    char **output = camelCaser(input);
    int result = checkIfMatch(output, exp);
    destroy(output);
    return result;
}

int whitespace(char**(*camelCaser)(const char*), void(*destroy)(char**)){
    char* input = "   spaceSpace	 SPACE   !   	 SPACE   sPACE.		";
    char* exp[] = {"spacespaceSpace","spaceSpace",NULL};
    char **output = camelCaser(input);
    int result = checkIfMatch(output, exp);
    destroy(output);
    return result;
}

int checkAllNums(char **(*camelCaser)(const char*), void(* destroy)(char**)){
    char* input = " 1 2 3 	. 4567 89. 012.  345 678.";
    char* exp[] = {"123", "456789", "012", "345678", NULL};
    char **output = camelCaser(input);
    int result = checkIfMatch(output,exp);
    destroy(output);
    return result;
}

int checkMultPunc(char **(*camelCaser)(const char*), void(* destroy)(char**)){
    char* input = "  abc dEF  . ! ? gHiJ\",. KLMNO..";
    char* exp[] = {"abcDef","","","ghij","","","klmno","",NULL};
    char **output = camelCaser(input);
    int result = checkIfMatch(output,exp);
    destroy(output);
    return result;
}

int checkOtherChars(char **(*camelCaser)(const char*), void(*destroy)(char**)){
    char* input = "@s#o &*abs  \n";
    char* exp[] = {"","s","o","",NULL};
    char **output = camelCaser(input);
    int result = checkIfMatch(output, exp);
    destroy(output);
    return result;
}

int testCombo(char **(*camelCaser)(const char*), void (*destroy)(char **)){
    char* input = "\\ \' \" \t    DE\nAD  \\   AT    41     YEARS  \\";
    char* exp[] = {"","","","deAd","at41Years",NULL};
    char **output = camelCaser(input);
    int result = checkIfMatch(output,exp);
    destroy(output);
    return result;
}

int testCaps(char** (*camelCaser)(const char*), void(*destroy)(char**)){
    char* input = " TODAY iS wEDnESDAY.! TOMORROW iS tHURSDAY!! PIZZA iS tHE bEST wHEN iTS cHEESE. i could use SOME pizza";
    char* exp[] = {"todayIsWednesday", "", "tomorrowIsThursday", "", "pizzaIsTheBestWhenItsCheese", NULL};
    char** output = camelCaser(input);
    int result = checkIfMatch(output, exp);
    destroy(output);
    return result;
}

int lettersAndNumbers(char **(*camelCaser)(const char*), void(*destroy)(char**)){
    char* input = "\t4MYlOVE\ni\rNEED\vyOU!\v7VEN\n11VEN\'L8TER\tG8TER\\";
    char* exp[] = {"4myloveINeedYou", "7ven11ven", "l8terG8ter", NULL};
    char** output = camelCaser(input);
    int result = checkIfMatch(output,exp);
    destroy(output);
    return result;
}
int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {

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
   if(!testCombo(camelCaser, destroy)){
	//printf("combo input failed\n");
	return 0;
   }
   if(!testNoPunc(camelCaser, destroy)){
	//printf("test no punc failed\n");
	return 0;
   }
   if(!testCaps(camelCaser, destroy)){
	//printrf("test caps failed\n");
	return 0;
   }
   if(!lettersAndNumbers(camelCaser,destroy)){
	return 0;
   }
   return 1;
}
