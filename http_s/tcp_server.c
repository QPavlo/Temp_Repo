#include <unistd.h>
#include <string.h>
#include "tcp_server.h"

#define BUFFER_SIZE 30000

int create_server(struct tcp_server *server, const char *ip_address, int port) {
    server->port = port;
    server->socket_address.sin_family = AF_INET;
    server->socket_address.sin_port = htons(port);
    server->socket_address_len = sizeof(server->socket_address);
    server->socket_address.sin_addr.s_addr = inet_addr(ip_address);
    strcpy(server->server_message, buildResponse());

    if (start_server(server) != 0) {
        printf("\n\nFailed to start server with PORT %d\n\n", port);
        return 1;
    }
    return 0;
}


int start_server(struct tcp_server *server) {
    server->serv_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server->serv_socket < 0) {
        printf("\n\nCannot create socket\n\n");
        return 1;
    }

    if (bind(server->serv_socket, (struct sockaddr *) &server->socket_address, server->socket_address_len) < 0) {
        printf("Cannot connect socket to address");
        return 1;
    }

    return 0;
}

void start_listen(struct tcp_server *server) {
    if (listen(server->serv_socket, 2000) < 0) {
        printf("Socket listen failed");
    }

    printf("\nListening on ADDRESS: %s PORT: %d\n",
           inet_ntoa(server->socket_address.sin_addr),
           ntohs(server->socket_address.sin_port));

    int bytesReceived;

    while (1) {
        printf("\n\nWaiting for a new connection\n\n\n");
        accept_connection(server);

        char buffer[BUFFER_SIZE] = {0};
        bytesReceived = read(server->client_socket, buffer, BUFFER_SIZE);

        if (bytesReceived < 0) {
            printf("Failed to read bytes from client socket connection");
        }

        printf("Received Request from client\n");

        send_response(server);
        close(server->client_socket);
    }
}

void accept_connection(struct tcp_server *server) {
    server->client_socket = accept(server->serv_socket, (struct sockaddr *) &server->socket_address,
                                   &server->socket_address_len);
    if (server->client_socket < 0) {

        printf("\n Server failed to accept incoming connection from ADDRESS: %s, PORT %d",
               inet_ntoa(server->socket_address.sin_addr),
               ntohs(server->socket_address.sin_port));
    }
}

const char *buildResponse() {
    return "Hello world from server";
}

void send_response(struct tcp_server *server) {
    long bytesSent;

    bytesSent = write(server->client_socket, server->server_message, strlen(server->server_message));

    if (bytesSent == strlen(server->server_message)) {
        printf("Server Response sent to client\n\n");
    } else {
        printf("Error sending response to client");
    }
}