//
// Created by pavlo on 27.02.23.
//

#ifndef UNTITLED1_REVERSE_DNS_H
#define UNTITLED1_REVERSE_DNS_H

#include <arpa/inet.h>
#include <netdb.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    STATUS_SUCCESS = 0,
    STATUS_WRONG_ARG_COUNT,
    STATUS_INV_IP,
    STATUS_GETNAMEINFO_ERROR,
} status;

static int convert_ipv4(struct sockaddr_in *sa4, const char *ip_addr_str);

static int convert_ipv6(struct sockaddr_in6 *sa6, const char *ip_addr_str);

int reverse_dns(const char *ip_addr_str, char *domain_name_str, uint32_t domain_name_length);

void log_error_info(status reverse_dns_status);

void log_error_info_str(const char *error_str);

#endif //UNTITLED1_REVERSE_DNS_H
