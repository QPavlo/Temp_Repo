#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#define MAX_REQUEST_SIZE 2048
#define SERVER_PORT 8080

// Function to write log entries to a file in the extended log format
void write_log_entry(char *time_stamp, char *method, char *uri, char *log_file_path) {
    // Open the log file for appending
    FILE *log_file = fopen(log_file_path, "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        exit(1);
    }

    long file_size = ftell(log_file);
    if (file_size == 0) {
        // Write the header to the file
        fprintf(log_file, "#Version: 1.0\n");
//        fprintf(log_file, "#Date: %s\n", time_stamp);
        fprintf(log_file, "#Fields: time cs-method cs-uri\n");
    }

    // Write the log message to the file
    fprintf(log_file, "%s %s %s\n", time_stamp, method, uri);

    // Close the log file
    fclose(log_file);
}

// Function to get the current time in the specified format
void get_current_time(char *buffer, size_t buffer_size, const char *format) {
    time_t raw_time;
    struct tm *time_info;
    char time_string[32];

    time(&raw_time);
    time_info = localtime(&raw_time);

    strftime(time_string, sizeof(time_string), format, time_info);
    snprintf(buffer, buffer_size, "%s", time_string);
}

int main(int argc, char *argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char request_buffer[MAX_REQUEST_SIZE] = {0};
    char *response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello world!";
    char time_stamp[32];
    char *method;
    char *uri;

    // Create a TCP socket for the server
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Failed to create server socket");
        exit(1);
    }

    // Set socket options to reuse the address and port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Failed to set socket options");
        exit(1);
    }

    // Bind the socket to a specific address and port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(SERVER_PORT);

    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("Failed to bind server socket");
        exit(1);
    }

    // Start listening for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Failed to listen for incoming connections");
        exit(1);
    }

    // Loop forever to handle incoming requests
    while (1) {
        // Accept an incoming connection and get the new socket file descriptor
        if ((new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t *) &addrlen)) < 0) {
            perror("Failed to accept incoming connection");
            exit(1);
        }

        // Read the incoming request from the socket
        if (read(new_socket, request_buffer, MAX_REQUEST_SIZE) < 0) {
            perror("Failed to read request from client");
            exit(1);
        }

        // Parse the request method and URI
        method = strtok(request_buffer, " ");
        uri = strtok(NULL, " ");

        // Get the current time
        // Get the current time
        get_current_time(time_stamp, sizeof(time_stamp), "%d-%b-%Y %H:%M:%S");

        // Write a log entry for the request
        write_log_entry(time_stamp, method, uri, "access.log");

        // Send the response back to the client
        send(new_socket, response, strlen(response), 0);

        // Close the socket
        close(new_socket);
    }

    return 0;
}