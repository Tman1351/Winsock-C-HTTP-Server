#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <ws2tcpip.h>


// constants
#define PORT 8000
#define BUFFER_SIZE 16000
#define BACKLOG 10

// Functions

// init winsock lib
int initialize_winsock();

// create new socket
SOCKET create_socket();

// bind socket
int bind_socket(SOCKET server_socket, int port);

// start listening for incomming connections
int start_listening(SOCKET server_socket);

// main server loop (handle client connections)
void handle_connections(SOCKET server_socket);

#endif