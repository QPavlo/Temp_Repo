#include "http_parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void push_back(struct http_header **head,
               char *name, char *value) {
    struct http_header *new_node = malloc(sizeof(struct http_header));

    new_node->name = calloc(strlen(name), sizeof(char));
    strcpy(new_node->name, name);

    new_node->value = calloc(strlen(value), sizeof(char));
    strcpy(new_node->value, value);

    struct http_header *temp_node;
    if (*head == NULL) {
        new_node->next = NULL;
        *head = new_node;
    } else {
        temp_node = *head;
        while (temp_node->next != NULL) {
            temp_node = temp_node->next;
        }
        temp_node->next = new_node;
        new_node->next = NULL;
    }
}

struct http_request *parse_request(const char *raw) {
    struct http_request *req = malloc(sizeof(struct http_request));

    if (!req) {
        return NULL;
    }

    memset(req, 0, sizeof(*req));

    char *raw_copy = malloc(10000);
    strcpy(raw_copy, raw);

    char *token = strtok(raw_copy, " ");

    if (strcmp(token, "GET") == 0) {
        req->method = GET;
    } else {
        req->method = UNSUPPORTED;
    }

    token = strtok(NULL, " ");
    strcpy(req->path, token);

    token = strtok(NULL, "\r\n");
    strcpy(req->version, token);


    struct list http_headers = {NULL, NULL};

    char *temp_header_name;
    char *temp_header_value;

    while (token[0] != '\r' && token[1] != '\n') {

        if ((token = strtok(NULL, ":")) == 0) {
            break;
        }
        temp_header_name = token + 1;

        if ((token = strtok(NULL, "\r\n")) == 0) {
            break;
        }

        temp_header_value = token + 1;
        push_back(&http_headers.head, temp_header_name, temp_header_value);
    }
    req->headers = http_headers.head;

    if (temp_header_name[0] == '\r' && temp_header_name[1] == '\n') {
        req->body = malloc(strlen(temp_header_name));
        strcpy(req->body, temp_header_name);
    }
    return req;
}

void free_header(struct http_header *header) {
    while (header) {
        free(header->name);
        free(header->value);
        header = header->next;
    }
    free(header);
}


void free_request(struct http_request *request) {
    free_header(request->headers);
    free(request->body);
    free(request);
}
