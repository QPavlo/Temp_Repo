#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define MAX_EVENTS 10
#define PORT 5000

int main() {
    int server_socket, new_socket, epoll_fd;
    struct sockaddr_in address;
    struct epoll_event event, events[MAX_EVENTS];

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    if ((epoll_fd = epoll_create1(0)) < 0) {
        perror("Epoll creation failed");
        exit(EXIT_FAILURE);
    }

    // Add server socket to epoll
    event.events = EPOLLIN;
    event.data.fd = server_socket;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &event) < 0) {
        perror("Epoll_ctl add failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

        for (int i = 0; i < num_events; i++) {
            if (events[i].data.fd == server_socket) {
                // New client connection
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);

                if ((new_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_len)) < 0) {
                    perror("Accept failed");
                    exit(EXIT_FAILURE);
                }

                // Add new client socket to epoll
                event.events = EPOLLIN;
                event.data.fd = new_socket;

                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &event) < 0) {
                    perror("Epoll_ctl add failed");
                    exit(EXIT_FAILURE);
                }

                printf("New client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            } else {
                // Data received from client
                printf("data received\n");
                int nbytes;
                char buffer[1024];
                char buffer1[1024] = {"HTTP/1.1 200 OK\n"
                                      "Date: Mon, 27 Jul 2009 12:28:53 GMT\n"
                                      "Server: Apache/2.2.14\n"
                                      "Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\n"
                                      "Content-Length: 88\n"
                                      "Content-Type: text/html\n"
                                      "Connection: Closed\n"
                                      "\n"
                                      "<html>\n"
                                      "<body>\n"
                                      "<h1>Hello, World!</h1>\n"
                                      "</body>\n"
                                      "</html>"};

                if ((nbytes = read(events[i].data.fd, buffer, sizeof(buffer))) <= 0) {
                    // Client disconnected
                    if (nbytes == 0) {
                        printf("Client disconnected\n");
                    } else {
                        perror("Read error");
                    }

                    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) < 0) {
                        perror("Epoll_ctl del failed");
                        exit(EXIT_FAILURE);
                    }

                    close(events[i].data.fd);
                } else {
                    // Print received data and echo it back to client
//                    buffer[nbytes] = '\0';
                    printf("Received data: %s\n", buffer);
                    write(events[i].data.fd, buffer1, strlen(buffer1));
                    close(events[i].data.fd);
                }
            }
        }
    }

    return 0;

}
