#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>

struct tcp_server {
    int port;
    int serv_socket;

    union {
        struct sockaddr sa;
        struct sockaddr_in s4;
        struct sockaddr_in6 s6;
    } addr;

    socklen_t socket_address_len;
    char server_message[3000];
};

int create_server(struct tcp_server *server, const char *ip_address, int port);

int start_server(struct tcp_server *server);

_Noreturn void start_listen(struct tcp_server *server);

void close_server(struct tcp_server *server);

int accept_connection(struct tcp_server *server);

const char *buildResponse();

void send_response(struct tcp_server *server, int client_socket);


#endif //TCP_SERVER_H
