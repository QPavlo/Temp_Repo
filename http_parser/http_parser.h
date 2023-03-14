#ifndef HTTP_PARSER_HTTP_PARSER_H
#define HTTP_PARSER_HTTP_PARSER_H

typedef enum Method {
    UNSUPPORTED, GET
} Method;

struct http_header {
    char *name;
    char *value;
    struct http_header *next;
};

struct list {
    struct http_header *head;
    struct http_header *last;
};

struct http_request {
    enum Method method;
    char path[2048];
    char version[16];
    struct http_header *headers;
    char *body;
};

struct http_request *parse_request(const char *raw);

void free_header(struct http_header *h);

void free_request(struct http_request *req);

void push_back(struct http_header **head,
               char *name, char *value);


#endif //HTTP_PARSER_HTTP_PARSER_H
