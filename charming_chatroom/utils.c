/**
 * Charming Chatroom
 * CS 241 - Spring 2020
 */
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "utils.h"
static const size_t MESSAGE_SIZE_DIGITS = 4;

char *create_message(char *name, char *message) {
    int name_len = strlen(name);
    int msg_len = strlen(message);
    char *msg = calloc(1, msg_len + name_len + 4);
    sprintf(msg, "%s: %s", name, message);

    return msg;
}

ssize_t get_message_size(int socket) {
    int32_t size;
    ssize_t read_bytes =
        read_all_from_socket(socket, (char *)&size, MESSAGE_SIZE_DIGITS);
    if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;

    return (ssize_t)ntohl(size);
}

// You may assume size won't be larger than a 4 byte integer
ssize_t write_message_size(size_t size, int socket) {
    // Your code here
    int32_t write_size = htonl(size);
    size_t num_written = write_all_to_socket(socket, (char*)&write_size, MESSAGE_SIZE_DIGITS);
    return num_written;
}

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

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
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
