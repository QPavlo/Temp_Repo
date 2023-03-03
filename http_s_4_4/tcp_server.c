#include <unistd.h>
#include <string.h>
#include "tcp_server.h"

#define BUFFER_SIZE 30000

int convert_ipv4(struct sockaddr_in *sa4, const char *ip_addr_str) {
    return inet_pton(sa4->sin_family = AF_INET, ip_addr_str, &sa4->sin_addr);
}

int convert_ipv6(struct sockaddr_in6 *sa6, const char *ip_addr_str) {
    return inet_pton(sa6->sin6_family = AF_INET6, ip_addr_str, &sa6->sin6_addr);
}


int create_server(struct tcp_server *server, const char *ip_address, int port) {

    if (convert_ipv4(&server->addr.s4, ip_address) == 1) {
        server->addr.s4.sin_port = htons(port);
    } else if (convert_ipv6(&server->addr.s6, ip_address) == 1) {
        server->addr.s6.sin6_port = htons(port);
    } else {
        printf("\n\nInvalid IP address\n\n");
    }

    server->port = port;
    server->socket_address_len = sizeof(server->addr);
    strcpy(server->server_message, buildResponse());

    if (start_server(server) != 0) {
        printf("\n\nFailed to start server with PORT %d\n\n", port);
        return 1;
    }
    return 0;
}


int start_server(struct tcp_server *server) {
    server->serv_socket = socket(server->addr.sa.sa_family, SOCK_STREAM, 0);
    if (server->serv_socket < 0) {
        printf("\n\nCannot create socket\n\n");
        return 1;
    }

    if (bind(server->serv_socket, (struct sockaddr *) &server->addr, server->socket_address_len) < 0) {
        printf("Cannot connect socket to address");
        return 1;
    }

    return 0;
}

void start_listen(struct tcp_server *server) {
    if (listen(server->serv_socket, 200) < 0) {
        printf("Socket listen failed");
    }

    char buff[400] = {0};
    inet_ntop(server->addr.sa.sa_family, &server->addr.s6.sin6_addr, buff, server->socket_address_len);
    printf("\n Listening on ADDRESS: %s PORT:  %d", buff, server->port);

    long bytesReceived;
    int client_socket;

    while (1) {
        printf("\n\nWaiting for a new connection\n\n\n");

        client_socket = accept_connection(server);

        char buffer[BUFFER_SIZE] = {0};
        bytesReceived = read(client_socket, buffer, BUFFER_SIZE);

        if (bytesReceived < 0) {
            printf("Failed to read bytes from client socket connection");
        }

        printf("Received Request from client\n");

        send_response(server, client_socket);
        close(client_socket);
    }
}

int accept_connection(struct tcp_server *server) {
    int client_socket = accept(server->serv_socket, (struct sockaddr *) &server->addr,
                               &server->socket_address_len);
    if (client_socket < 0) {
        char buff[400] = {0};
        inet_ntop(server->addr.sa.sa_family, server->addr.sa.sa_data, buff, server->socket_address_len);
        printf("\n Server failed to accept incoming connection from ADDRESS: %s, PORT %d",
               buff,
               server->port);
    }
    return client_socket;
}

const char *buildResponse() {
    return "Hello world from server";
}

void send_response(struct tcp_server *server, int client_socket) {
    long bytesSent = write(client_socket, server->server_message, strlen(server->server_message));;

    if (bytesSent == strlen(server->server_message)) {
        printf("Server Response sent to client\n\n");
    } else {
        printf("Error sending response to client");
    }
}