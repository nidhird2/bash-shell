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
#include <signal.h>

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
void handle_get_response(char*);
void handle_list_response();
void handle_error();


void connection_ended(){
    print_connection_closed();
    exit(1);
}
//Returns char* array in form of {host, port, method, remote, local, NULL}
int main(int argc, char **argv) {
    signal(SIGPIPE, connection_ended); 

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

void handle_error(){
    size_t to_read;
    size_t bad_req_len = strlen(err_bad_request);
    size_t no_such_file = strlen(err_no_such_file);
    size_t bad_file_len = strlen(err_bad_file_size);
    if(bad_req_len > no_such_file && bad_req_len > bad_file_len){
        to_read = bad_req_len;
    }
    else if (no_such_file > bad_req_len && no_such_file > bad_file_len){
        to_read = no_such_file;
    }
    else{
        to_read = bad_file_len;
    }
    char buffer[to_read + 1];
    size_t bytes_read = read_all_from_socket(SOCKET_FD, buffer, to_read);
    if(bytes_read == 0){
        return print_invalid_response();
    }
    if(buffer[bytes_read - 1] != '\n'){
        print_invalid_response();
    }
    //strip last newline
    buffer[bytes_read - 1] = '\0';
    print_error_message(buffer);
}

void handle_response(char** parsed_args, verb method){
    if(method == GET){
        handle_get_response(parsed_args[4]);
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
    char* error = "ERROR\n";
    char buffer[strlen(error) + 1];
    size_t bytes_read = read_all_from_socket(SOCKET_FD, buffer, 3);
    //reponse is invalid!
    if(bytes_read < strlen(good)){
        return print_invalid_response();
    }
    //check and print error!
    buffer[bytes_read] = '\0';
    //response did not return OK
    if(strcmp(good, buffer) != 0){
        bytes_read = read_all_from_socket(SOCKET_FD, buffer + 3, 3);
        buffer[bytes_read + 3] = '\0';
        //response did not return OK or ERROR
        if(strcmp(error, buffer) != 0){
            return print_invalid_response();
        }
        return handle_error();
    }
    //response was ok!
    print_success();
}

void handle_list_response(){
    char* good = "OK\n";
    char* error = "ERROR\n";
    char buffer[strlen(error) + 1];
    size_t bytes_read = read_all_from_socket(SOCKET_FD, buffer, 3);
    buffer[bytes_read] = '\0';
    //reponse is invalid!
    if(bytes_read < strlen(good)){
        return print_invalid_response();
    }
    //check and print error!
    buffer[bytes_read] = '\0';
    //response did not return OK
    if(strcmp(good, buffer) != 0){
        bytes_read = read_all_from_socket(SOCKET_FD, buffer + 3, 3);
        buffer[bytes_read + 3] = '\0';
        //response did not return OK or ERROR
        if(strcmp(error, buffer) != 0){
            return print_invalid_response();
        }
        return handle_error();
    }
    //response was ok!
    size_t list_len;
    size_t temp = read_all_from_socket(SOCKET_FD, (char*)&list_len, sizeof(size_t));

    size_t buffer_size = 1000;
    char response_buffer[buffer_size + 1];
    bytes_read = 0;

    while(bytes_read < list_len){
        temp = read_all_from_socket(SOCKET_FD, response_buffer, buffer_size);
        //no more left to read!
        if(temp == 0){
            break;
        }
        bytes_read += temp;
        response_buffer[temp] = '\0';
        printf("%s", response_buffer);
    }
    //too little data!
    if(bytes_read < list_len){
        return print_too_little_data();
    }
    bytes_read += read_all_from_socket(SOCKET_FD, response_buffer, list_len + 1);
    //too much!
    if(bytes_read > list_len){
        return print_received_too_much_data();
    }
    return;
}

void handle_get_response(char* local_name){
    char* good = "OK\n";
    char* error = "ERROR\n";
    char buffer[strlen(error) + 1];
    size_t bytes_read = read_all_from_socket(SOCKET_FD, buffer, 3);
    buffer[bytes_read] = '\0';
    //reponse is invalid!
    if(bytes_read < strlen(good)){
        return print_invalid_response();
    }
    //check and print error!
    buffer[bytes_read] = '\0';
    //response did not return OK
    if(strcmp(good, buffer) != 0){
        bytes_read = read_all_from_socket(SOCKET_FD, buffer + 3, 3);
        buffer[bytes_read + 3] = '\0';
        //response did not return OK or ERROR
        if(strcmp(error, buffer) != 0){
            return print_invalid_response();
        }
        return handle_error();
    }
    //response was ok!
    size_t list_len;
    size_t temp = read_all_from_socket(SOCKET_FD, (char*)&list_len, sizeof(size_t));

    size_t buffer_size = 1000;
    char response_buffer[buffer_size + 1];
    bytes_read = 0;

    FILE * f = fopen(local_name, "w");
    //unable to open file!
    if(!f){
        exit(1);
    }

    while(bytes_read < list_len){
        temp = read_all_from_socket(SOCKET_FD, response_buffer, buffer_size);
        //no more left to read!
        if(temp == 0){
            break;
        }
        bytes_read += temp;
        response_buffer[temp] = '\0';
        fwrite(response_buffer, 1, temp, f);
    }
    fclose(f);
    //too little data!
    if(bytes_read < list_len){
        return print_too_little_data();
    }
    bytes_read += read_all_from_socket(SOCKET_FD, response_buffer, list_len + 1);
    //too much!
    if(bytes_read > list_len){
        return print_received_too_much_data();
    }
    return;
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
    char* local_name = parsed_args[4];
    //open file and check that it exists
    FILE* f = fopen(local_name, "r");
    //local file does not exist!
    if(!f){
        exit(1);
    }
    char* basis = "PUT ";
    char* remote_name = parsed_args[3];
    char* newline = "\n";
    write_all_to_socket(SOCKET_FD, basis, strlen(basis));
    write_all_to_socket(SOCKET_FD, remote_name, strlen(remote_name));
    write_all_to_socket(SOCKET_FD, newline, strlen(newline));

    //move fd to end, count total size
    fseek (f, 0, SEEK_END);
    size_t length = ftell(f);
    fseek (f, 0, SEEK_SET);
    write_all_to_socket(SOCKET_FD, (char*)&length, sizeof(size_t));
    //move fd to beginning, start reading
    size_t buffer_size = 1000;
    char buffer[buffer_size + 1];
    size_t total_bytes_read = 0;
    while(total_bytes_read < length){
        size_t bytes_read = fread(buffer, 1, buffer_size, f);
        if(bytes_read == 0){
            break;
        }
        total_bytes_read += bytes_read;
        buffer[bytes_read] = '\0';
        size_t bytes_written = write_all_to_socket(SOCKET_FD, buffer, bytes_read);
        if(bytes_written < bytes_read){
            print_connection_closed();
            exit(1);
        }
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
        exit(1);
    }
    int optval = 1;
    int res = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if(res == -1){
        exit(1);
    }
    optval = 1;
    res = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    if(res == -1){
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
