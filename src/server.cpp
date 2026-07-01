#include <iostream>
/*
 * File: server.cpp
 * Description: RSE Server Main Process. The "Traffic Cop".
 * Loops infinitely binding a TCP socket, accepting clients, and routing 
 * their text strings to the appropriate backend module in srvfuncs.h.
 * Author: Originator
 */
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "common.h"
#include "logger.h"
#include "srvfuncs.h"

int  initSocket();
int  bindAndListen(int server_fd);
int  acceptClient(int server_fd);
void dispatch(const char* cmd, char* result);
void handleClientLoop(int client_fd);

// ==========================================
// The Main Server Loop
// ==========================================
int main() {
    int server_fd = initSocket();
    if (server_fd < 0) return 1;

    if (bindAndListen(server_fd) < 0) return 1;

    LOG_INFO("Server listening on port %d", PORT);

    // Single-Client Blocking Server:
    // It will completely freeze on acceptClient(). When a client connects, 
    // handleClientLoop() runs endlessly until they hang up.
    // If a second client tries to connect, they are queued in the OS BACKLOG.
    while (true) {
        int client_fd = acceptClient(server_fd);
        if (client_fd < 0) continue;
        
        handleClientLoop(client_fd);
        close(client_fd); // Safely hang up the network pipeline
        
        LOG_INFO0("Client disconnected. Waiting for next connection...");
    }

    close(server_fd);
    return 0;
}

// ==========================================
// Network Setup
// ==========================================
int initSocket() {
    // AF_INET = IPv4. SOCK_STREAM = TCP (Reliable, ordered delivery).
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        LOG_FATAL0("socket() failed");
        return -1;
    }

    // Networking Edge Case: SO_REUSEADDR
    // If the server crashes, the OS locks port 5678 in TIME_WAIT for 60 seconds.
    // This forcibly bypasses the lock allowing immediate restarts during dev.
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    LOG_INFO("Socket created (fd=%d)", fd);
    return fd;
}

int bindAndListen(int server_fd) {
    struct sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;  // Bind to ALL available network interfaces (WiFi, Ethernet, Localhost)
    addr.sin_port        = htons(PORT); // Host-To-Network Short: Flips binary Endianness for processor compatibility

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        LOG_FATAL("bind() failed on port %d", PORT);
        return -1;
    }

    if (listen(server_fd, BACKLOG) < 0) {
        LOG_FATAL0("listen() failed");
        return -1;
    }

    return 0;
}

int acceptClient(int server_fd) {
    struct sockaddr_in client_addr{};
    socklen_t len = sizeof(client_addr);

    // Program sleeps permanently here until a 3-way TCP handshake completes
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len);
    if (client_fd < 0) {
        LOG_WARN0("accept() failed");
        return -1;
    }

    // inet_ntop (Network To Pointer): Converts binary IP address back into printable "192.168.x.x" string
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    LOG_INFO("Client connected from %s", client_ip);

    return client_fd;
}

// ==========================================
// The Router / Dispatcher
// ==========================================
// Parses the string "COMMAND:ARG1:ARG2" and executes the correct backend function.
void dispatch(const char* cmd, char* result) {
    // String character search: Find the very first colon delimiter.
    const char* delim = strchr(cmd, ':');
    // If a colon exists, the argument starts exactly 1 character after it. Else, empty string.
    const char* arg   = delim ? delim + 1 : "";

    // strncmp: Compares the left side of the colon with the hardcoded Macros from common.h
    if (strncmp(cmd, CMD_SEARCH_FILE, strlen(CMD_SEARCH_FILE)) == 0) {
        LOG_INFO("Dispatching CMD_SEARCH_FILE for: %s", arg);
        rse_srv_searchForFile(arg, result);

    } else if (strncmp(cmd, CMD_FIND_FILE, strlen(CMD_FIND_FILE)) == 0) {
        LOG_INFO("Dispatching CMD_FIND_FILE for: %s", arg);
        
        // strncpy: Safely copy the string so strtok doesn't mutate the original read-only parameter
        char arg_copy[BUFFER_SIZE];
        strncpy(arg_copy, arg, BUFFER_SIZE - 1);
        arg_copy[BUFFER_SIZE - 1] = '\0';

        // strtok splits the multi-argument string (e.g. "/etc:passwd")
        char* base_path = strtok(arg_copy, CMD_DELIM);
        char* target_filename = strtok(nullptr, CMD_DELIM);

        // Defensive checks preventing Null Pointer crashes
        if (base_path && target_filename) {
            rse_srv_findFile(base_path, target_filename, result);
        } else {
            LOG_WARN0("Invalid CMD_FIND_FILE format");
            snprintf(result, BUFFER_SIZE, "Error: Invalid command format for FIND_FILE");
        }

    } else if (strncmp(cmd, CMD_SEARCH_STR, strlen(CMD_SEARCH_STR)) == 0) {
        LOG_INFO("Dispatching SEARCH_STR for: %s", arg);
        char arg_copy[BUFFER_SIZE];
        strncpy(arg_copy, arg, BUFFER_SIZE - 1);
        arg_copy[BUFFER_SIZE - 1] = '\0';

        char* base_path = strtok(arg_copy, CMD_DELIM);
        char* query = strtok(nullptr, CMD_DELIM);

        if (base_path && query) {
            rse_srv_searchForString(base_path, query, result);
        } else {
            LOG_WARN0("Invalid SEARCH_STR format");
            snprintf(result, BUFFER_SIZE, "Error: Invalid command format for SEARCH_STR");
        }

    } else if (strncmp(cmd, CMD_DISPLAY, strlen(CMD_DISPLAY)) == 0) {
        LOG_INFO("Dispatching DISPLAY for: %s", arg);
        rse_srv_displayFileContent(arg, result);

    } else if (strcmp(cmd, CMD_EXIT) == 0) {
        LOG_INFO0("Client requested EXIT");
        snprintf(result, BUFFER_SIZE, "BYE");

    } else {
        LOG_WARN("Unknown command: %s", cmd);
        snprintf(result, BUFFER_SIZE, "Error: Unknown command");
    }
}

// ==========================================
// Client Interaction Loop
// ==========================================
void handleClientLoop(int client_fd) {
    char cmd[BUFFER_SIZE];
    char result[BUFFER_SIZE];

    while (true) {
        // Prevent ghost data from leaking between requests
        memset(cmd, 0, sizeof(cmd));
        memset(result, 0, sizeof(result));

        // Wait to receive the command. 
        // len - 1 mathematically guarantees the buffer ends with a \0 terminator
        ssize_t bytes = recv(client_fd, cmd, sizeof(cmd) - 1, 0);
        
        // If 0, the client disconnected smoothly. If -1, the connection dropped ungracefully.
        if (bytes <= 0) {
            LOG_WARN("recv() returned %zd — client likely disconnected", bytes);
            break; 
        }

        LOG_DEBUG("Received command: %s", cmd);

        // The Brains
        dispatch(cmd, result);

        // Send the fully populated answer over the wire back to the client
        send(client_fd, result, strlen(result), 0);

        // Stop serving this client if they requested a termination
        if (strcmp(cmd, CMD_EXIT) == 0) break;
    }
}
