#include "reverse_dns.h"

int main(int argc, char **argv) {
    char domain_name[NI_MAXHOST] = {0};
    int status;

    if (argc != 2) {
        log_error_info(STATUS_WRONG_ARG_COUNT);
        return EXIT_FAILURE;
    }

    status = reverse_dns(argv[1], domain_name, sizeof(domain_name));

    if (status) {
        log_error_info(status);
        if (status == STATUS_GETNAMEINFO_ERROR) {
            log_error_info_str(domain_name);
        }
        return EXIT_FAILURE;
    }

    printf("Reverse DNS information for IP address %s:\n", argv[1]);
    printf("Domain name: %s\n", domain_name);

    return EXIT_SUCCESS;
}