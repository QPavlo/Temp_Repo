#include "reverse_dns.h"


const char *STATUS_DETAILS[] = {"Success",
                                "Wrong number of arguments\nUsage: %s [IP]\nE.g. ./reverse_dns 10.32.129.77\n",
                                "Invalid IP address",
                                "getnameinfo error"};

int convert_ipv4(struct sockaddr_in *sa4, const char *ip_addr_str) {
    return inet_pton(sa4->sin_family = AF_INET, ip_addr_str, &sa4->sin_addr);
}

int convert_ipv6(struct sockaddr_in6 *sa6, const char *ip_addr_str) {
    return inet_pton(sa6->sin6_family = AF_INET6, ip_addr_str, &sa6->sin6_addr);
}

void log_error_info(status reverse_dns_status) {
    fprintf(stderr, "status: %s\n", STATUS_DETAILS[reverse_dns_status]);
}

void log_error_info_str(const char *error_str) {
    fprintf(stderr, "error: %s\n", error_str);
}

int reverse_dns(const char *ip_addr_str, char *domain_name_str, uint32_t domain_name_length) {

    union {
        struct sockaddr sa;
        struct sockaddr_in s4;
        struct sockaddr_in6 s6;
    } addr;

    int res;

    if (convert_ipv4(&addr.s4, ip_addr_str) != 1 && convert_ipv6(&addr.s6, ip_addr_str) != 1) {
        return STATUS_INV_IP;
    }

    res = getnameinfo(&addr.sa, sizeof(addr), domain_name_str, domain_name_length, NULL, 0, NI_NAMEREQD);
    if (res) {
        strcpy(domain_name_str, gai_strerror(res));
        return STATUS_GETNAMEINFO_ERROR;
    }

    return STATUS_SUCCESS;
}