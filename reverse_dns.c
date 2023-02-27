#include "reverse_dns.h"

int convertIPv4(struct sockaddr_in *sa, const char *name) {
    return inet_pton(sa->sin_family = AF_INET, name, &sa->sin_addr);
}

int convertIPv6(struct sockaddr_in6 *sa, const char *name) {
    return inet_pton(sa->sin6_family = AF_INET6, name, &sa->sin6_addr);
}


int start_app(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [IP]\nE.g. %s 10.32.129.77\n", argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    union {
        struct sockaddr sa;
        struct sockaddr_in s4;
        struct sockaddr_in6 s6;
    } addr;

    if (convertIPv4(&addr.s4, argv[1]) != 1 && convertIPv6(&addr.s6, argv[1]) != 1) {
        fprintf(stderr, "%s: not a valid IP address.\n", argv[1]);
        return EXIT_FAILURE;
    }

    char domain_name[NI_MAXHOST] = {0};
    int res = getnameinfo(&addr.sa, sizeof(addr), domain_name, sizeof(domain_name), NULL, 0, NI_NAMEREQD);

    if (res) {
        fprintf(stderr, "%s: %s\n", argv[1], gai_strerror(res));
        return EXIT_FAILURE;
    }

    printf("Reverse DNS information for IP address %s:\n", argv[1]);
    printf("Domain name: %s\n", domain_name);
    return EXIT_SUCCESS;
}