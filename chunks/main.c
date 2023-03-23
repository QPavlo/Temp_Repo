#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_PORT 8080
#define SERVER_ADDR "127.0.0.1"
#define FILE_PATH "Image.jpg"


int main() {

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.
            sin_family = AF_INET;
    server_address.sin_addr.
            s_addr = inet_addr(SERVER_ADDR);
    server_address.
            sin_port = htons(SERVER_PORT);

    int bind_result = bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if (bind_result < 0) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    int listen_result = listen(server_socket, 10);
    if (listen_result < 0) {
        perror("Failed to listen on socket");
        exit(EXIT_FAILURE);
    }


    while (1) {
        struct sockaddr_in client_address;
        socklen_t client_address_size = sizeof(client_address);
        int client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_size);
        if (client_socket < 0) {
            perror("Failed to accept connection");
            continue;
        }

        char request[4096];
        int bytes_received = recv(client_socket, request, sizeof(request), 0);
        if (bytes_received < 0) {
            perror("Failed to receive request");
            close(client_socket);
            continue;
        }

        char *response_header = "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\nContent-Type: image/jpeg\r\n\r\n";
        int response_header_size = strlen(response_header);
        if (send(client_socket, response_header, response_header_size, 0) != response_header_size) {
            perror("Failed to send response header");
            close(client_socket);
            continue;
        }

        FILE *file = fopen(FILE_PATH, "rb");
        int file_descriptor = fileno(file);

        if (file_descriptor < 0) {
            perror("Failed to open file");
            close(client_socket);
            continue;
        }

        char chunk_size[32];
        while (1) {
            char buffer[4096];
            int bytes_read = read(file_descriptor, buffer, sizeof(buffer));
            if (bytes_read < 0) {
                perror("Failed to read from file");
                close(file_descriptor);
                close(client_socket);
                break;
            }

            if (bytes_read == 0) {
                send(client_socket, "0\r\n\r\n", 5, 0);
                close(file_descriptor);
                close(client_socket);
                break;
            }

            sprintf(chunk_size, "%x\r\n", bytes_read);
            int chunk_size_size = strlen(chunk_size);

            if (send(client_socket, chunk_size, chunk_size_size, 0) != chunk_size_size) {
                perror("Failed to send chunk size");
                close(file_descriptor);
                close(client_socket);
                break;
            }

            if (send(client_socket, buffer, bytes_read, 0) != bytes_read) {
                perror("Failed to send chunk data");
                close(file_descriptor);
                close(client_socket);
                break;
            }

            if (send(client_socket, "\r\n", 2, 0) != 2) {
                perror("Failed to send chunk end");
                close(file_descriptor);
                close(client_socket);
                break;
            }
        }
    }
}