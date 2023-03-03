#include "tcp_server.h"

int main() {
    struct tcp_server server;
    int status = create_server(&server, "127.0.0.1", 5000);
    if (status) {
        return EXIT_FAILURE;
    }

    start_listen(&server);

    return 0;
}
