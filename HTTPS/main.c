#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define PORT 443

int main(int argc, char **argv) {
    SSL_CTX *ctx;
    SSL *ssl;
    int server_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    // initialize SSL library
    SSL_library_init();

    // create SSL context
    ctx = SSL_CTX_new(TLSv1_2_server_method());
    if (!ctx) {
        fprintf(stderr, "Failed to create SSL context.\n");
        return EXIT_FAILURE;
    }

    // load server certificate and private key
    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0) {
        fprintf(stderr, "Failed to load server certificate.\n");
        return EXIT_FAILURE;
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0) {
        fprintf(stderr, "Failed to load server private key.\n");
        return EXIT_FAILURE;
    }

    // create socket and bind to port
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        fprintf(stderr, "Failed to create socket.\n");
        return EXIT_FAILURE;
    }
    memset(&addr, 0, len);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&addr, len) == -1) {
        fprintf(stderr, "Failed to bind to port %d.\n", PORT);
        return EXIT_FAILURE;
    }

    // listen for incoming connections
    if (listen(server_fd, SOMAXCONN) == -1) {
        fprintf(stderr, "Failed to listen for incoming connections.\n");
        return EXIT_FAILURE;
    }

    printf("HTTPS server listening on port %d...\n", PORT);

    // accept incoming connections and handle them
    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&addr, &len);
        if (client_fd == -1) {
            fprintf(stderr, "Failed to accept incoming connection.\n");
            continue;
        }

        // create SSL connection
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client_fd);
        if (SSL_accept(ssl) <= 0) {
            fprintf(stderr, "Failed to establish SSL connection.\n");
            SSL_free(ssl);
            close(client_fd);
            continue;
        }

        // handle request
        char buffer[1024];
        SSL_read(ssl, buffer, sizeof(buffer));
        printf("Received request:\n%s\n", buffer);

        char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, world!\n";
        SSL_write(ssl, response, strlen(response));

        // close SSL connection and socket
        SSL_free(ssl);
        close(client_fd);
    }

    // cleanup
    SSL_CTX_free(ctx);

    return EXIT_SUCCESS;
}

//openssl req -x509 -newkey rsa:4096 -nodes -keyout server.key -out server.crt -days 365

