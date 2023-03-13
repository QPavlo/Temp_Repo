#include "http_parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct Request *parse_request(const char *raw) {
    struct Request *req = NULL;
    req = malloc(sizeof(struct Request));
    if (!req) {
        return NULL;
    }
    memset(req, 0, sizeof(struct Request));

    size_t meth_len = strcspn(raw, " ");
    if (memcmp(raw, "GET", strlen("GET")) == 0) {
        req->method = GET;
    } else if (memcmp(raw, "HEAD", strlen("HEAD")) == 0) {
        req->method = HEAD;
    } else {
        req->method = UNSUPPORTED;
    }
    raw += meth_len + 1;

    size_t path_len = strcspn(raw, " ");
    req->path = malloc(path_len + 1);
    if (!req->path) {
        free_request(req);
        return NULL;
    }
    memcpy(req->path, raw, path_len);
    req->path[path_len] = '\0';
    raw += path_len + 1;

    size_t ver_len = strcspn(raw, "\r\n");
    req->version = malloc(ver_len + 1);
    if (!req->version) {
        free_request(req);
        return NULL;
    }
    memcpy(req->version, raw, ver_len);
    req->version[ver_len] = '\0';
    raw += ver_len + 2;

    struct Header *head = NULL, *last = NULL;
    while (raw[0] != '\r' || raw[1] != '\n') {
        last = head;
        head = malloc(sizeof(*head));
        if (!head) {
            free_request(req);
            return NULL;
        }

        size_t name_len = strcspn(raw, ":");
        head->name = malloc(name_len + 1);
        if (!head->name) {
            free_request(req);
            return NULL;
        }
        memcpy(head->name, raw, name_len);
        head->name[name_len] = '\0';
        raw += name_len + 1;
        while (*raw == ' ') {
            raw++;
        }

        size_t value_len = strcspn(raw, "\r\n");
        head->value = malloc(value_len + 1);
        if (!head->value) {
            free_request(req);
            return NULL;
        }
        memcpy(head->value, raw, value_len);
        head->value[value_len] = '\0';
        raw += value_len + 2;

        // next
        head->next = last;
    }
    req->headers = head;
    raw += 2;

    size_t body_len = strlen(raw);
    req->body = malloc(body_len + 1);
    if (!req->body) {
        free_request(req);
        return NULL;
    }
    memcpy(req->body, raw, body_len);
    req->body[body_len] = '\0';

    return req;
}


void free_header(struct Header *h) {
    while (h) {
        free(h->name);
        free(h->value);
        h = h->next;
    }
    free(h);
}


void free_request(struct Request *req) {
    free(req->path);
    free(req->version);
    free_header(req->headers);
    free(req->body);
    free(req);
}
