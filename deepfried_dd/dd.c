/**
 * Deepfried dd
 * CS 241 - Spring 2020
 */
#include "format.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h> 

static int PRINT_IT = 0;

void siguser_handle(){
    PRINT_IT = 1;
}

void print_progress(struct timespec start, size_t full_blocks_in, size_t partial_blocks_in,
                         size_t full_blocks_out, size_t partial_blocks_out,
                         size_t total_bytes_copied){
    struct timespec finish;
    clock_gettime(CLOCK_MONOTONIC, &finish); 
    double seconds = (double)finish.tv_sec - (double)start.tv_sec;
    double ns = (double)finish.tv_nsec - (double)start.tv_nsec; 
    double dur = seconds + ns/1000000000;
    print_status_report(full_blocks_in, partial_blocks_in, full_blocks_out, partial_blocks_out,
                        total_bytes_copied, dur);
}

// -i <file>: input file (defaults to stdin)
// -o <file>: output file (defaults to stdout)
// You should create this file if does not already exist.
// -b <size>: block size, the number of bytes copied at a time (defaults to 512)
// -c <count>: total number of blocks copied (defaults to the entire file)
// -p <count>: number of blocks to skip at the start of the input file (defaults to 0)
// -k <count>: number of blocks to skip at the start of the output file (defaults to 0)

int main(int argc, char **argv) {
    signal(SIGUSR1, siguser_handle); 

    FILE *input = stdin, *output = stdout;
    size_t block_size = 512;
    size_t total_blocks_to_copy = 0;
    size_t block_skip_input = 0;
    size_t block_skip_output = 0;

    char* input_filename = NULL, *output_filename = NULL;

    char* options = "i:o:b:c:p:k:";
    int opt = getopt(argc, argv, options);
    while(opt != -1){
        if(opt == 'i'){
            input_filename = optarg;

        }
        else if(opt == 'o'){
            output_filename = optarg;
        }
        else if(opt == 'b'){
            sscanf(optarg, "%lu", &block_size);
        }
        else if(opt == 'c'){
            sscanf(optarg, "%lu", &total_blocks_to_copy);
        }
        else if(opt == 'p'){
            //block_skip_input = atoi(optarg);
            sscanf(optarg, "%lu", &block_skip_input);
        }
        else if(opt == 'k'){
            //block_skip_output = atoi(optarg);
            sscanf(optarg, "%lu", &block_skip_output);
        }
        else{
            exit(1);
        }
        opt = getopt(argc, argv, options);
    }
    printf("input filename: %s\n", input_filename);
    printf("output filename: %s\n", output_filename);
    printf("block size: %lu\n", block_size);
    printf("# blocks to read: %lu\n", total_blocks_to_copy);
    printf("block skip input: %lu\n", block_skip_input);
    printf("block skip output: %lu\n", block_skip_output);
    if(input_filename != NULL){
        input = fopen(input_filename, "r");
        if(input == NULL){
            print_invalid_input(input_filename);
            exit(1);
        }
    }
    if(output_filename != NULL){
        output = fopen(output_filename, "r+");
        if(output == NULL){
            print_invalid_output(input_filename);
            exit(1);
        }
    }

    struct timespec start; 
    clock_gettime(CLOCK_MONOTONIC, &start); 
    if(input != stdin){
        fseek(input, block_skip_input * block_size, SEEK_SET);
    } else {
        size_t num_char = (block_skip_input * block_size) + 1;
        char buf[num_char];
        fgets(buf,num_char, stdin);
    }
    if(output != stdout){
        fseek(output, block_skip_output * block_size, SEEK_SET);
    }

    size_t blocks_copied = 0;

    // size_t block_size = 512;
    // size_t total_blocks_to_copy = -1;
    // size_t block_skip_input = 0;
    // size_t block_skip_output = 0;
    size_t partial_copied = 0;
    size_t total_bytes = 0;
    size_t bytes_read = 1;
    char buff[block_size + 1];

    while(1){
        if(PRINT_IT == 1){
            print_progress(start, blocks_copied, partial_copied, blocks_copied, partial_copied, total_bytes);
            PRINT_IT = 0;
        }
        bytes_read = fread(buff, 1, block_size, input);
        total_bytes += bytes_read;
        if(bytes_read < block_size && bytes_read > 0){
            partial_copied += 1;
        }
        else{
            blocks_copied += 1;
        }
        bytes_read = fwrite(buff, 1, bytes_read, output);
        //copied enough!
        if(blocks_copied + partial_copied == total_blocks_to_copy){
            break;
        }
        //reached EOF, stop copying
        if(feof(input)){
            break;
        }
    }
    print_progress(start, blocks_copied, partial_copied, blocks_copied, partial_copied, total_bytes);
    return 0;
}