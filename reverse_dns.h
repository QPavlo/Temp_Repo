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

static int convertIPv4(struct sockaddr_in *sa, const char *IPv4);

static int convertIPv6(struct sockaddr_in6 *sa, const char *IPv6);

int start_app(int argc, char **argv);

#endif //UNTITLED1_REVERSE_DNS_H
