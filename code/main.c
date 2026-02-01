#include <stdio.h>
#include "server.h"

int main() {

    SOCKET server_socket;

    printf("Windows HTTP Server in C\n");

    if (initialize_winsock() != 0) {
        return 1;
    }

    server_socket = create_socket();
    if (server_socket == INVALID_SOCKET) {
        WSACleanup();
        return 1;
    }

    if (bind_socket(server_socket, PORT) != 0) {
        WSACleanup();
        return 1;
    }
    
    if (start_listening(server_socket) != 0) {
        WSACleanup();
        return 1;
    }

    handle_connections(server_socket);

    closesocket(server_socket);
    WSACleanup();

    return 0;
}