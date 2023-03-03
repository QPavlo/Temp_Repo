#include "tcp_server.h"

//0000:0000:0000:0000:0000:0000:0000:0001
//127.0.0.1
//[::1]
int main() {
    struct tcp_server server;
//    int status = create_server(&server, "127.0.0.1", 5000);
    int status = create_server(&server, "0000:0000:0000:0000:0000:0000:0000:0001", 5000);
    if (status) {
        return EXIT_FAILURE;
    }

    start_listen(&server);

    return 0;
}
