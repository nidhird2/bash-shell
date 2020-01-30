/**
 * Perilous Pointers
 * CS 241 - Spring 2020
 */
#include "part2-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * (Edit this function to print out the "Illinois" lines in
 * part2-functions.c in order.)
 */
int main() {
    // your code here
    first_step(81);
    int val = 132;
    second_step(&val);
    val = 8942;
    int* p = &val;
    double_step(&p);
    char* ex= "00000\x0f\0\0\0\0";
    strange_step(ex);
    char* exp[] = {"a", "a", "a", "\0", "\0"};
    empty_step((void*)exp);
    char* b = "uuuuuuuuuuuu";
    two_step((void*)b, b);
    three_step(b, b+2, b+4);
    char* aa = "AAAAAAAAAAAA";
    char* bb ="IIIIIIIIIIIIIIIIJJJJ"; 
    char* cc = "QQQQQQQQQQQQQQQ";
    step_step_step(aa,bb,cc); 
    char* ii= "I";
    it_may_be_odd(ii, 73);
    char last[]= "CS241,CS241, CS241, CS241";
    tok_step(last);
    char* orb = "\x01\x0B\0\0";
    the_end((void*)orb, (void*)orb);
    return 0;
}
