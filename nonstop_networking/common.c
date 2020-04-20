/**
 * Nonstop Networking
 * CS 241 - Spring 2020
 */
#include "common.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "format.h"
//static const size_t MESSAGE_SIZE_DIGITS = 4;


//directly from my charming chatroom lab!
ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    // Your Code Here
    size_t num_read = 0;

    while(num_read < count){
        ssize_t res = read(socket, buffer + num_read, count - num_read);
        if(res == 0){
            return num_read;
        }
        else if (res > 0){
            num_read += res;

        }
        else if (res == -1 && errno == EINTR){
            //try again
            continue;
        }
        else{
            return num_read;
        }
    }
    return num_read;
}

//directly from my charming chatroom lab!
ssize_t write_all_to_socket(int socket, char *buffer, size_t count) {
    // Your Code Here
    size_t num_written = 0;

    while(num_written < count){
        ssize_t res = write(socket, buffer + num_written, count - num_written);
        if(res == 0){
            return num_written;
        }
        else if(res > 0){
            num_written += res;
        }
        else if (res == -1 && errno == EINTR){
            continue;
        }
        else{
            return num_written;
        }
    }
    return num_written;
}
