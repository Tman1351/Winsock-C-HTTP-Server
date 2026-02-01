#ifndef ROUTER_H
#define ROUTER_H

#include <winsock.h>

// http request struct
typedef struct httpReq_t {
    char method[10];
    char path[256];
    char version[10]; // HTTP/1.1
    char query[512];
    char body[4096];  
} httpReq_t;

// router handler func type
typedef void (*RouteHandler)(SOCKET client_socket, httpReq_t *request);

// route struct
typedef struct route_t{
    char method[10];        
    char path[256]; 
    RouteHandler handler; 
} route_t;

// func prototypes

int parse_http_response(const char *raw_request, httpReq_t *request);

int route_request(SOCKET client_socket, httpReq_t *request);

void add_route(const char *method, const char *path, RouteHandler handler);

void init_router();

void send_response(SOCKET client_socket, int status_code);

void send_json_response(SOCKET client_socket, const char *json);

void send_html_response(SOCKET client_socket, const char *html);

void send_404(SOCKET client_socket);

#endif