/**
 * Nonstop Networking
 * CS 241 - Spring 2020
 */
#include "common.h"
#include "format.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>

static int SOCKET_FD;
//static int MAX_HEADER_SIZE = 1024;
char **parse_args(int argc, char **argv);
verb check_args(char **args);
int connect_to_server(const char *host, const char *port);

void send_request(char** parse_args, verb method);
void create_delete_request(char** parsed_args);
void create_get_request(char** parsed_args);
void create_put_request(char** parsed_args);
void create_list_request(char** parsed_args);

void handle_response(char** parsed_args, verb method);
void handle_put_delete_response();
void handle_get_response();
void handle_list_response();

//Returns char* array in form of {host, port, method, remote, local, NULL}
int main(int argc, char **argv) {
    // Good luck!
    char** parsed_args = parse_args(argc, argv);
    verb method = check_args(parsed_args);
    //connect to server
    SOCKET_FD = connect_to_server(parsed_args[0], parsed_args[1]);
    //build str for server req -> process local file into bytes (PUT)
    send_request(parsed_args, method);
    //shutdown write of socket
    shutdown(SOCKET_FD, SHUT_WR);
    //read server response
    //handle server esponse -> process bytes into local file (GET)
    handle_response(parsed_args, method);
    close(SOCKET_FD);
    return 0;
}

void handle_response(char** parsed_args, verb method){
    if(method == GET){
        handle_get_response();
    } 
    else if (method == LIST){
        handle_list_response();
    }
     else{
        handle_put_delete_response();
    }
}

void handle_put_delete_response(){
    char* good = "OK\n";
    char buffer[strlen(good) + 1];
    size_t bytes_read = read_all_from_socket(SOCKET_FD, buffer, strlen(good));
    buffer[bytes_read] = '\0';
    if(strcmp(good, buffer)){
        return print_success();
    }
    //check and print error!
}

void handle_list_response(){
    char* list = "LIST\n";
    char buffer[strlen(list) + 1];
    size_t bytes_read = read_all_from_socket(SOCKET_FD, buffer, strlen(list));
    buffer[bytes_read] = '\0';
    if(!strcmp(list, buffer)){
        return print_invalid_response();
    }
    size_t list_len;
    read_all_from_socket(SOCKET_FD, (char*)&list_len, sizeof(size_t));
    char response_buffer[list_len + 1];
    bytes_read = read_all_from_socket(SOCKET_FD, response_buffer, list_len + 1);
    //too little data!
    if(bytes_read < list_len){
        return print_too_little_data();
    }
    if(bytes_read > list_len){
        return print_received_too_much_data();
    }
    response_buffer[list_len + 1] = '\0';
    printf("%s", response_buffer);
    return;
}

void handle_get_response(){
    
}
//build req for delete request
void create_delete_request(char** parsed_args){
    char* basis = "DELETE ";
    char* remote = parsed_args[3];
    char* newline = "\n";
    write_all_to_socket(SOCKET_FD, basis, strlen(basis));
    write_all_to_socket(SOCKET_FD, remote, strlen(remote));
    write_all_to_socket(SOCKET_FD, newline, strlen(newline));
}

void create_get_request(char** parsed_args){
    char* basis = "GET ";
    char* remote = parsed_args[3];
    char* newline = "\n";
    write_all_to_socket(SOCKET_FD, basis, strlen(basis));
    write_all_to_socket(SOCKET_FD, remote, strlen(remote));
    write_all_to_socket(SOCKET_FD, newline, strlen(newline));
}

void create_list_request(char** parsed_args){
    char* basis = "LIST\n";
    write_all_to_socket(SOCKET_FD, basis, strlen(basis));
}

//Returns char* array in form of {host, port, method, remote, local, NULL}
void create_put_request(char** parsed_args){
    char* basis = "PUT ";
    char* remote_name = parsed_args[3];
    char* newline = "\n";
    write_all_to_socket(SOCKET_FD, basis, strlen(basis));
    write_all_to_socket(SOCKET_FD, remote_name, strlen(remote_name));
    write_all_to_socket(SOCKET_FD, newline, strlen(newline));
    char* local_name = parsed_args[4];

    FILE* f = fopen(local_name, "r");
    //local file does not exist!
    if(!f){
        exit(1);
    }
    //move fd to end, count total size
    fseek (f, 0, SEEK_END);
    size_t length = ftell(f);
    write_all_to_socket(SOCKET_FD, (char*)&length, sizeof(size_t));
    //move fd to beginning, start reading
    fseek (f, 0, SEEK_SET);
    size_t buffer_size = 1000;
    char buffer[buffer_size];
    size_t total_bytes_read = 0;
    while(total_bytes_read < length){
        total_bytes_read += fread(buffer, 1, buffer_size, f);
        write_all_to_socket(SOCKET_FD, buffer, buffer_size);
    }
    fclose(f);
}



void send_request(char** parsed_args, verb method){
    if(method == GET){
        create_get_request(parsed_args);
    }
    else if(method == PUT){
        create_put_request(parsed_args);
    }
    else if(method == DELETE){
        create_delete_request(parsed_args);
    }
    else{
        create_list_request(parsed_args);
    }
}

//attempts to connect to server
int connect_to_server(const char *host, const char *port) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd == -1){
        print_connection_closed();
        exit(1);
    }
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));

    int s = getaddrinfo (host, port, &hints, &result);
    if (s != 0) {
        //fprintf(stderr, "%s\n", gai_strerror(s));
        print_connection_closed();
        exit(1);
    }
    if(connect(sock_fd, result->ai_addr, result->ai_addrlen) != 0){
        print_connection_closed();
        exit(1);
    }
    freeaddrinfo(result);
    return sock_fd;
}

/**
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
    if (argc < 3) {
        return NULL;
    }

    char *host = strtok(argv[1], ":");
    char *port = strtok(NULL, ":");
    if (port == NULL) {
        return NULL;
    }

    char **args = calloc(1, 6 * sizeof(char *));
    args[0] = host;
    args[1] = port;
    args[2] = argv[2];
    char *temp = args[2];
    while (*temp) {
        *temp = toupper((unsigned char)*temp);
        temp++;
    }
    if (argc > 3) {
        args[3] = argv[3];
    }
    if (argc > 4) {
        args[4] = argv[4];
    }

    return args;
}

/**
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args) {
    if (args == NULL) {
        print_client_usage();
        exit(1);
    }

    char *command = args[2];

    if (strcmp(command, "LIST") == 0) {
        return LIST;
    }

    if (strcmp(command, "GET") == 0) {
        if (args[3] != NULL && args[4] != NULL) {
            return GET;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "DELETE") == 0) {
        if (args[3] != NULL) {
            return DELETE;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "PUT") == 0) {
        if (args[3] == NULL || args[4] == NULL) {
            print_client_help();
            exit(1);
        }
        return PUT;
    }

    // Not a valid Method
    print_client_help();
    exit(1);
}
