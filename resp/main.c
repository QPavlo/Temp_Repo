#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct http_header {
    char *name;
    char *value;
    struct http_header *next;
};

struct list {
    struct http_header *head;
    struct http_header *last;
};

struct http_response {
    char version[16];
    int status_code;
    char *status_message;
    struct list headers;
    char *body;
};

void push_back(struct list *head,
               const char *name, const char *value) {
    struct http_header *new_node = malloc(sizeof(struct http_header));

    new_node->name = calloc(strlen(name), sizeof(char));
    strcpy(new_node->name, name);

    new_node->value = calloc(strlen(value), sizeof(char));
    strcpy(new_node->value, value);

    if (head->head == NULL) {
        head->head = new_node;
        head->last = new_node;
    } else {
        head->last->next = new_node;
        head->last = head->last->next;
    }
}

void add_header(struct http_response *response, const char *name, const char *value) {
    push_back(&response->headers, name, value);
}

void set_body(struct http_response *response, const char *body_text) {
    response->body = malloc(strlen(body_text));
    strcpy(response->body, body_text);
    strcpy(response->body, "\n");
}

void set_version(struct http_response *response, const char *version) {
    strcpy(response->version, version);
}

char *raw_resp(struct http_response resp) {
    char *raw_resp = malloc(10000);


    sprintf(raw_resp, "%s %d %s\n", resp.version, resp.status_code, resp.status_message);

    for (struct http_header *temp = resp.headers.head; temp; temp = temp->next) {
        strcat(raw_resp, temp->name);
        strcat(raw_resp, ":");
        strcat(raw_resp, temp->value);
        strcat(raw_resp, "\n");

    }
    strcat(raw_resp, "\n");


    if (resp.body) {
        strcat(raw_resp, resp.body);
    }

    return raw_resp;
}


int main() {
    struct http_response response;

    memset(&response, 0, sizeof(response));

    set_version(&response, "HTTP/1.1");
    response.status_code = 200;
    response.status_message = "OK";

    add_header(&response, "Server", "Apache");

    char *res = raw_resp(response);
    printf("%s", res);

    return 0;
}
