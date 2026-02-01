#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"
#include "logger.h"

// link wisock
#pragma comment(lib, "ws2_32.lib")

// init winsock
int initialize_winsock() {
    WSADATA wsaData;
    GINFO("Initializing Winsock")

    int iResult;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        GFAILURE("Error Code: %d", WSAGetLastError());
        return 1;
    }

    GINFO("Winsock initialized");
    return 0;
}

// create new socket
SOCKET create_socket() {
    SOCKET server_socket;

    GINFO("Creating socket");
    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (server_socket == INVALID_SOCKET) {
        GFAILURE("Could not create socket: %d", WSAGetLastError());
        return INVALID_SOCKET;
    }

    GINFO("Socket created");
    return server_socket;
}

// bind socket
int bind_socket(SOCKET server_socket, int port) {
    struct sockaddr_in server_addr;

    // clear structure
    memset(&server_addr, 0, sizeof(server_addr));

    // set up address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // bind socket
    GINFO("Binding socket");

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {

        GERROR("Bind failed with error code: %d", WSAGetLastError());
        return 1;
    }

    GINFO("Socket bound successfully");
    return 0;
}

// start listening
int start_listening(SOCKET server_socket) {
    GINFO("Listening for connections");

    if (listen(server_socket, BACKLOG) == SOCKET_ERROR) {
        GERROR("Listen failed with error code: %d", WSAGetLastError());
        closesocket(server_socket);
        return 1;
    }

    GINFO("Server is now listening on port %d", PORT);
    GINFO("Ctrl + Click: http://localhost:%d", PORT);
    return 0;
}

void handle_connections(SOCKET server_socket) {
    SOCKET client_socket;
    struct sockaddr_in client_addr;
    int client_addr_size;
    char buffer[BUFFER_SIZE];
    int bytes_received;

    // Generate the complete HTTP response with headers
    char http_response[8000]; 
    
    // Create HTTP headers
    const char* http_headers = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n"; 
    
    // Create HTML body
    const char* html_body = 
        "<!doctype html>\r\n"
        "<html lang=\"en\">\r\n"
        "  <head>\r\n"
        "    <meta charset=\"UTF-8\" />\r\n"
        "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />\r\n"
        "    <title>Server Ready</title>\r\n"
        "    <style>\r\n"
        "      * {\r\n"
        "        margin: 0;\r\n"
        "        padding: 0;\r\n"
        "        box-sizing: border-box;\r\n"
        "      }\r\n"
        "\r\n"
        "      body {\r\n"
        "        font-family:\r\n"
        "          -apple-system, BlinkMacSystemFont, \"Segoe UI\", Roboto, sans-serif;\r\n"
        "        background: #0f0f0f;\r\n"
        "        color: #fff;\r\n"
        "        min-height: 100vh;\r\n"
        "        display: flex;\r\n"
        "        align-items: center;\r\n"
        "        justify-content: center;\r\n"
        "        padding: 20px;\r\n"
        "      }\r\n"
        "\r\n"
        "      .terminal {\r\n"
        "        background: #1a1a1a;\r\n"
        "        border-radius: 12px;\r\n"
        "        padding: 40px;\r\n"
        "        max-width: 600px;\r\n"
        "        width: 100%;\r\n"
        "        border: 1px solid #333;\r\n"
        "        box-shadow: 0 20px 60px rgba(0, 0, 0, 0.4);\r\n"
        "        position: relative;\r\n"
        "      }\r\n"
        "\r\n"
        "      .prompt {\r\n"
        "        color: #00ff88;\r\n"
        "        font-family: \"Monaco\", \"Consolas\", monospace;\r\n"
        "        font-size: 14px;\r\n"
        "        margin-bottom: 2px;\r\n"
        "      }\r\n"
        "\r\n"
        "      .prompt-yellow {\r\n"
        "        color: #ffff00;\r\n"
        "        font-family: \"Monaco\", \"Consolas\", monospace;\r\n"
        "        font-size: 14px;\r\n"
        "      }\r\n"
        "\r\n"
        "      .prompt-grey {\r\n"
        "        color: #888;\r\n"
        "        font-family: \"Monaco\", \"Consolas\", monospace;\r\n"
        "        font-size: 14px;\r\n"
        "      }\r\n"
        "\r\n"
        "      .status {\r\n"
        "        font-size: 28px;\r\n"
        "        font-weight: 500;\r\n"
        "        margin: 30px 0 20px 0;\r\n"
        "        letter-spacing: -0.5px;\r\n"
        "      }\r\n"
        "\r\n"
        "      .status .live {\r\n"
        "        color: #00ff88;\r\n"
        "      }\r\n"
        "\r\n"
        "      .info-line {\r\n"
        "        color: #888;\r\n"
        "        font-size: 14px;\r\n"
        "        margin: 8px 0;\r\n"
        "        font-family: \"Monaco\", \"Consolas\", monospace;\r\n"
        "      }\r\n"
        "\r\n"
        "      .port {\r\n"
        "        color: #00a8ff;\r\n"
        "        font-weight: 600;\r\n"
        "      }\r\n"
        "\r\n"
        "      .protocol {\r\n"
        "        color: #ff6b9d;\r\n"
        "        font-weight: 600;\r\n"
        "      }\r\n"
        "\r\n"
        "      .divider {\r\n"
        "        height: 1px;\r\n"
        "        background: linear-gradient(90deg, transparent, #333, transparent);\r\n"
        "        margin: 30px 0;\r\n"
        "      }\r\n"
        "\r\n"
        "      .footer {\r\n"
        "        font-size: 12px;\r\n"
        "        color: #666;\r\n"
        "        text-align: center;\r\n"
        "        margin-top: 50px;\r\n"
        "        padding-top: 20px;\r\n"
        "        border-top: 1px solid #222;\r\n"
        "        font-family: \"Monaco\", \"Consolas\", monospace;\r\n"
        "      }\r\n"
        "\r\n"
        "      .log-box {\r\n"
        "        background: rgba(0, 0, 0, 0.2);\r\n"
        "        padding: 15px;\r\n"
        "        border-radius: 6px;\r\n"
        "        margin: 10px 0;\r\n"
        "        font-family: \"Monaco\", \"Consolas\", monospace;\r\n"
        "        font-size: 14px;\r\n"
        "        color: #888;\r\n"
        "        line-height: 1.5;\r\n"
        "      }\r\n"
        "    </style>\r\n"
        "  </head>\r\n"
        "  <body>\r\n"
        "    <div class=\"terminal\">\r\n"
        "      <div class=\"log-box\">\r\n"
        "        <div class=\"timestamp\">[2026-01-30 23:46:13]</div>\r\n"
        "        <br />\r\n"
        "        <span class=\"prompt-yellow\">\r\n"
        "          user@Name MINGW64 ~/Desktop/Coding/C/http_server\r\n"
        "        </span>\r\n"
        "\r\n"
        "        <div class=\"prompt\">$ ./http_server.exe</div>\r\n"
        "        Welcome to your Windows HTTP Server in C<br />\r\n"
        "        [INFO] Initializing Winsock<br />\r\n"
        "        [INFO] Winsock initialized<br />\r\n"
        "        [INFO] Creating socket<br />\r\n"
        "        [INFO] Socket created<br />\r\n"
        "        [INFO] Binding socket<br />\r\n"
        "        [INFO] Socket bound successfully<br />\r\n"
        "        [INFO] Listening for connections<br />\r\n"
        "        [INFO] Server is now listening on port %d<br />\r\n"
        "        [INFO] Ctrl + Click:\r\n"
        "        <a\r\n"
        "          href=\"http://localhost:%d\"\r\n"
        "          style=\"color: #00a8ff; text-decoration: none\"\r\n"
        "          >http://localhost:%d</a>\r\n"
        "      </div>\r\n"
        "\r\n"
        "      <div class=\"status\">SERVER <span class=\"live\">ONLINE</span></div>\r\n"
        "\r\n"
        "      <div class=\"info-line\">[✓] HTTP server initialized</div>\r\n"
        "      <div class=\"info-line\">\r\n"
        "        [✓] Socket bound to port <span class=\"port\">%d</span>\r\n"
        "      </div>\r\n"
        "      <div class=\"info-line\">[✓] Listening for connections</div>\r\n"
        "      <div class=\"info-line\">\r\n"
        "        [✓] Protocol: <span class=\"protocol\">TCP/IP</span>\r\n"
        "      </div>\r\n"
        "\r\n"
        "      <div class=\"divider\"></div>\r\n"
        "\r\n"
        "      <div class=\"info-line\">> Port: %d</div>\r\n"
        "      <div class=\"info-line\">> Connection backlog: %d</div>\r\n"
        "      <div class=\"info-line\">> Max buffer size: %dKB</div>\r\n"
        "\r\n"
        "      <div class=\"footer\">\r\n"
        "        &copy; Tamanveer Dhillon 2026 • WinSocket C HTTP Server\r\n"
        "      </div>\r\n"
        "    </div>\r\n"
        "  </body>\r\n"
        "</html>\r\n";
    
    // Combine headers + formatted HTML body
    snprintf(http_response, sizeof(http_response), 
        "%s%s", http_headers, html_body);
    
    // Format the HTML with actual constants
    char final_response[10000];
    int content_length = snprintf(final_response, sizeof(final_response), http_response, PORT, PORT, PORT, PORT, PORT, BACKLOG, BUFFER_SIZE/1024);
    

    while (1) {
        client_addr_size = sizeof(client_addr);

        GINFO("Waiting for connection...");

        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);

        if (client_socket == INVALID_SOCKET) {
            GERROR("Accept failed with error code: %d", WSAGetLastError());
            continue;
        }

        GINFO("Connection accepted");

        bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            GINFO("Recived %d bytes:\n%s", bytes_received, buffer);

            send(client_socket, final_response, strlen(final_response), 0);
            GINFO("Response sent to client");
        } else if (bytes_received == 0) {
            GINFO("Connection closing");
        } else {
            GERROR("recv failed with error: %d", WSAGetLastError());
        }

        closesocket(client_socket);

    }
}