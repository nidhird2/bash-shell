/**
 * Extreme Edge Cases
 * CS 241 - Spring 2020
 */
#include <stdio.h>

#include "camelCaser_ref_utils.h"

int main() {
    // Enter the string you want to test with the reference here.
    char *input = "hello. welcome to cs241";

    // This function prints the reference implementation output on the terminal.
    print_camelCaser(input);

    char* input2 = "\t4MYlOVE\ni\rNEED\vyOU!\v7VEN\n11VEN\'L8TER\tG8TER\v";
    print_camelCaser(input2);

    // Put your expected output for the given input above.
    char *correct[] = {"hello", NULL};
    char *wrong[] = {"hello", "welcomeToCs241", NULL};

    // Compares the expected output you supplied with the reference output.
    printf("check_output test 1: %d\n", check_output(input, correct));
    printf("check_output test 2: %d\n", check_output(input, wrong));

    // Feel free to add more test cases.
    return 0;
}
