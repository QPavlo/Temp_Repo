#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

int main(int argc, char *argv[]) {
    struct in6_addr ipv6addr;
    struct in_addr ipv4addr;
    struct hostent *host;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s [IP]\n", argv[0]);
        return 1;
    }

    if (inet_pton(AF_INET6, argv[1], &ipv6addr) > 0) { // IPv6 address
        host = gethostbyaddr(&ipv6addr, sizeof ipv6addr, AF_INET6);
    } else if (inet_pton(AF_INET, argv[1], &ipv4addr) > 0) { // IPv4 address
        host = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
    } else {
        fprintf(stderr, "Invalid IP address: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    if (host == NULL) {
        fprintf(stderr, "Could not retrieve reverse DNS information\n");
        return EXIT_FAILURE;
    }

    printf("Reverse DNS information for IP address %s:\n", argv[1]);
    printf("Domain name: %s\n", host->h_name);

    return EXIT_SUCCESS;
}