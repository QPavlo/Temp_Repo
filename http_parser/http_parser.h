#ifndef HTTP_PARSER_HTTP_PARSER_H
#define HTTP_PARSER_HTTP_PARSER_H

typedef enum Method {
    UNSUPPORTED, GET, HEAD
} Method;

typedef struct Header {
    char *name;
    char *value;
    struct Header *next;
} Header;

typedef struct Request {
    enum Method method;
    char *path;
    char *version;
    struct Header *headers;
    char *body;
} Request;


struct Request *parse_request(const char *raw);

void free_header(struct Header *h);

void free_request(struct Request *req);


#endif //HTTP_PARSER_HTTP_PARSER_H
