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
//    printf("sentences: %d, total chars: %d\n", num_sentences, totalChars);
    char* wordSpace = (char*)malloc(totalChars);
    result = (char**)malloc(sizeof(char*)*(num_sentences+1));
    if(num_sentences == 0){
	free(wordSpace);
	wordSpace = NULL;
    }
    const char* current = input_str;
    char* currentSpace = wordSpace;
    int isPreviousSpace = 1;
    int isFirstWord = 1;
    int toUp = 0;
    int currentIdx = 0;
    while(*current){
	//printf("current: %s\n", current);
	if(currentIdx == num_sentences){
		break;
	}
	if(isspace(*current)){
		isPreviousSpace = 1;
		current++;
		if(!isFirstWord){
			toUp = 1;
		}
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
		toUp = 0;
		current++;
	}
	else{
		if(toUp && isalpha(*current)){
			*currentSpace = (char)toupper(*current);
			toUp = 0;
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
    if(result == NULL){
	return;
    }
    free(result[0]);
    free(result);
    result = NULL;
    return;
}
