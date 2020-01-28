/**
 * Extreme Edge Cases
 * CS 241 - Spring 2020
 */
#include "camelCaser.h"
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

int countSentences(const char* input);
int countChars(const char* input);

int countSentences(const char *input){
    int count = 0;
    const char* current = input;
    while(*current){
	if(ispunct(*current)){
		count++;
	}
	current++;
    }
    return count;
}

int countChars(const char *input){
    int count = 0;
    int running = 0;
    const char* current = input;
    while(*current){
	if(isspace(*current)){
		current++;
		continue;
	}
	else if(ispunct(*current)){
		count += running + 1;
		running = 0;
		current++;
	}
	else{
		running++;
		current++;
	}
    }
    return count;
}

char **camel_caser(const char *input_str) {
    // TODO: Implement me!
    char** result = NULL;
    if(input_str == NULL){
	return result;
    }
    int num_sentences = countSentences(input_str);
    int totalChars = countChars(input_str);
    char* wordSpace = (char*)malloc(totalChars);
    result = (char**)malloc(sizeof(char*)*(num_sentences+1));
    const char* current = input_str;
    char* currentSpace = wordSpace;
    int isPreviousSpace = 1;
    int isFirstWord = 1;
    int currentIdx = 0;
    while(*current){
	if(isspace(*current)){
		isPreviousSpace = 1;
		current++;
		continue;
	}
	else if(ispunct(*current)){
		*currentSpace = '\0';
		currentSpace++;
		result[currentIdx] = wordSpace;
		currentIdx++;
		wordSpace = currentSpace;
		isFirstWord = 1;
		isPreviousSpace = 1;
		current++;
	}
	else{
		if(isPreviousSpace && (!isFirstWord)){
			*currentSpace = (char)toupper(*current);
		}
		else{
			*currentSpace = (char)tolower(*current);
		}
		isFirstWord = 0;
		isPreviousSpace = 0;
		currentSpace++;
		current++; 
	}
    }
    result[currentIdx] = NULL;
    return result;
    
}

void destroy(char **result) {
    // TODO: Implement me!
    if(result == NULL){
	return;
    }
    free(result[0]);
    free(result);
    result = NULL;
    return;
}
