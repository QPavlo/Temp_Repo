#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/epoll.h>

#define MAX_EVENTS 10
#define MAX_CLIENTS 10

typedef struct {
    int fd;
    struct sockaddr_in addr;
} client_info;

pthread_mutex_t mutex;

void *handle_client(void *arg);


int main(int argc, char *argv[]) {
    int listen_fd, conn_fd, epoll_fd, nready, i;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    struct epoll_event ev, events[MAX_EVENTS];

    // Create listening socket
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options to allow reuse of address and port
    int optval = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Bind socket to address and port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);
    if (bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(listen_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    // Create epoll instance
    if ((epoll_fd = epoll_create1(0)) < 0) {
        perror("Epoll creation failed");
        exit(EXIT_FAILURE);
    }

    // Add listening socket to epoll
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev) < 0) {
        perror("Epoll_ctl failed");
        exit(EXIT_FAILURE);
    }

    // Create thread pool
    pthread_t threads[MAX_CLIENTS];
    int thread_count = 0;

    // Event loop
    while (1) {
        nready = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nready < 0) {
            perror("Epoll_wait failed");
            exit(EXIT_FAILURE);
        }

        // Handle events
        for (i = 0; i < nready; i++) {
            if (events[i].data.fd == listen_fd) {
                // New client connection
                if ((conn_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_len)) < 0) {
                    perror("Accept failed");
                    continue;
                }

                // Set socket to non-blocking mode
                if (fcntl(conn_fd, F_SETFL, fcntl(conn_fd, F_GETFL, 0) | O_NONBLOCK) < 0) {
                    perror("Fcntl failed");
                    close(conn_fd);
                    continue;
                }

                // Add client socket to epoll
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_fd;


                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev) < 0) {
                    perror("Epoll_ctl failed");
                    close(conn_fd);
                    continue;
                }

                // Create new thread to handle client connection
                client_info *info = malloc(sizeof(client_info));
                info->fd = conn_fd;
                info->addr = client_addr;
                if (pthread_create(&threads[thread_count++], NULL, handle_client, (void *) info) != 0) {
                    perror("Thread creation failed");
                    close(conn_fd);
                    free(info);
                    continue;
                }

                if (thread_count >= MAX_CLIENTS) {
                    thread_count = 0;
                    printf("Maximum client limit reached. Closing listening socket.\n");
//                    close(listen_fd);
//                    exit(EXIT_FAILURE);
                }
            } else {
                // Existing client socket has data to read
                pthread_mutex_lock(&mutex);
                ev = events[i];
                conn_fd = ev.data.fd;
                pthread_mutex_unlock(&mutex);
                client_info *info = malloc(sizeof(client_info));
                info->fd = conn_fd;
                info->addr = client_addr;
                if (pthread_create(&threads[thread_count++], NULL, handle_client, (void *) info) != 0) {
                    perror("Thread creation failed");
                    close(conn_fd);
                    free(info);
                    continue;
                }
            }
        }
    }

// Cleanup
    close(listen_fd);
    close(epoll_fd);
    return 0;

}

void *handle_client(void *arg) {
    client_info *info = (client_info *) arg;
    int conn_fd = info->fd;
    struct sockaddr_in client_addr = info->addr;
    free(info);


    char buffer[1024];
    ssize_t nread;

// Read client data
    if ((nread = read(conn_fd, buffer, sizeof(buffer))) > 0) {
        if (write(conn_fd, buffer, nread) < 0) {
            perror("Write failed");
            close(conn_fd);
            return NULL;
        }
    }

    if (nread == 0) {
        printf("Client %s:%d disconnected\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    } else {
        perror("Read failed");
    }

// Cleanup
    close(conn_fd);
    return NULL;

}