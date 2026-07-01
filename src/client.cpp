/*
 * File: client.cpp
 * Description: RSE Client interactive menu and network interface.
 * Abstracts pure TCP network sockets away from the user behind a clean menu loop.
 * Author: Originator
 */
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "common.h"
#include "logger.h"

int  rse_cli_connectToServer(const char* p_host);
void rse_cli_showMenu();
int  rse_cli_getUserChoice();
void rse_cli_sendCommand(int sockfd, const char* p_cmd);
void rse_cli_receiveResponse(int sockfd, char* p_out, int len);
void rse_cli_handleSearchFile(int sockfd);
void rse_cli_handleSearchString(int sockfd);
void rse_cli_handleDisplayContent(int sockfd);

// ==========================================
// Main Execution Engine
// ==========================================
int main() {
    int sockfd = rse_cli_connectToServer("127.0.0.1");
    if (sockfd < 0) {
        LOG_FATAL("Could not connect to server at 127.0.0.1:%d", PORT);
        return 1; // Program dies immediately if server isn't running
    }

    LOG_INFO("Connected to server on port %d", PORT);

    // Infinite loop presenting the UI until the user selects Exit.
    while (true) {
        rse_cli_showMenu();
        int choice = rse_cli_getUserChoice();

        if (choice == 1)      rse_cli_handleSearchFile(sockfd);
        else if (choice == 2) rse_cli_handleSearchString(sockfd);
        else if (choice == 3) rse_cli_handleDisplayContent(sockfd);
        else if (choice == 4) {
            // Polite termination: We don't just abruptly sever the socket.
            // We tell the server dispatch loop we are disconnecting so it can clean up.
            rse_cli_sendCommand(sockfd, CMD_EXIT);
            LOG_INFO0("Disconnecting from server");
            break;
        } else {
            std::cout << "Invalid choice. Please enter 1-4.\n";
        }
    }

    close(sockfd); // Release OS network resources
    return 0;
}

// ==========================================
// TCP Socket Logic
// ==========================================
int rse_cli_connectToServer(const char* p_host) {
    // AF_INET = IPv4. SOCK_STREAM = TCP socket.
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        LOG_FATAL0("socket() failed");
        return -1;
    }

    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(PORT); // Host to Network Short: corrects byte endianness

    // Parses human-readable IP "127.0.0.1" into binary bytes for the OS socket struct
    if (inet_pton(AF_INET, p_host, &server_addr.sin_addr) <= 0) {
        LOG_FATAL("Invalid server address: %s", p_host);
        return -1;
    }

    // Attempt the 3-Way TCP Handshake
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        LOG_FATAL0("connect() failed — is the server running?");
        return -1;
    }

    return sockfd;
}

// UI Wrapper
void rse_cli_showMenu() {
    std::cout << "\n=============================\n";
    std::cout << "  Remote Search Engine (RSE)\n";
    std::cout << "=============================\n";
    std::cout << "  1. Search for a file\n";
    std::cout << "  2. Search for a string in files\n";
    std::cout << "  3. Display file contents\n";
    std::cout << "  4. Exit\n";
    std::cout << "=============================\n";
    std::cout << "Enter choice: ";
}

// Input Safety Wrapper
int rse_cli_getUserChoice() {
    int choice = 0;
    // Exploit Defense: If a user types the string "HELLO" into this INT expectation,
    // standard cin enters a permanently broken fail state causing infinite loop spam.
    if (!(std::cin >> choice)) {
        std::cin.clear();              // Erase broken state flags
        std::cin.ignore(10000, '\n');  // Flush the garbage out of the buffer pipeline
        return 0; // Returns 0, which correctly hits the "Invalid choice" logic block
    }
    std::cin.ignore(10000, '\n'); // clear leftover newline from hitting Enter
    return choice;
}

// Network Write/Read Wrappers
void rse_cli_sendCommand(int sockfd, const char* p_cmd) {
    LOG_DEBUG("Sending: %s", p_cmd);
    send(sockfd, p_cmd, strlen(p_cmd), 0);
}

void rse_cli_receiveResponse(int sockfd, char* p_out, int len) {
    memset(p_out, 0, len);
    // Buffer security: len - 1 prevents overflow and guarantees a null-terminator.
    recv(sockfd, p_out, len - 1, 0);
    LOG_DEBUG("Received response (%zu bytes)", strlen(p_out));
}

// ==========================================
// Handlers (String formatters)
// ==========================================
void rse_cli_handleSearchFile(int sockfd) {
    std::cout << "\n--- Search for File ---\n";
    std::cout << "1. By absolute path\n";
    std::cout << "2. By file name\n";
    std::cout << "Enter choice: ";
    
    int choice = rse_cli_getUserChoice();
    
    if (choice == 1) {
        std::cout << "Enter absolute file path: ";
        char path[MAX_PATH];
        std::cin.getline(path, MAX_PATH);

        char cmd[MAX_PATH + 32];
        // Formatting human input into the strict COMMON protocol logic e.g., "SEARCH_FILE:/etc/passwd"
        snprintf(cmd, sizeof(cmd), "%s%s%s", CMD_SEARCH_FILE, CMD_DELIM, path);
        rse_cli_sendCommand(sockfd, cmd);

        char result[BUFFER_SIZE];
        rse_cli_receiveResponse(sockfd, result, BUFFER_SIZE);
        std::cout << "\n" << result << "\n";
    } else if (choice == 2) {
        std::cout << "Enter optional base path (default is ./): ";
        char base_path[MAX_PATH];
        std::cin.getline(base_path, MAX_PATH);
        // Dynamic Input Handling: Automatically insert root path if explicitly skipped by user
        if (strlen(base_path) == 0) {
            strcpy(base_path, "./");
        }

        std::cout << "Enter file name to find: ";
        char filename[MAX_PATH];
        std::cin.getline(filename, MAX_PATH);

        char cmd[MAX_PATH * 2 + 32];
        snprintf(cmd, sizeof(cmd), "%s%s%s%s%s", CMD_FIND_FILE, CMD_DELIM, base_path, CMD_DELIM, filename);
        rse_cli_sendCommand(sockfd, cmd);

        char result[BUFFER_SIZE];
        rse_cli_receiveResponse(sockfd, result, BUFFER_SIZE);
        
        if (strncmp(result, "No matches", 10) == 0 || strncmp(result, "Error", 5) == 0) {
            std::cout << "\n" << result << "\n";
            return; // Gracefully escapes without parsing empty/error strings
        }

        // ------------------
        // Advanced Interactive Parsing
        // ------------------
        std::cout << "\n--- Search Results ---\n";
        int line_count = 0;
        char path_array[100][MAX_PATH];
        
        // Breaks massive payload by "\n"
        char* token = strtok(result, "\n");
        // Security logic: line_count < 100 prevents multidimensional array limits from causing a SegFault
        while (token != nullptr && line_count < 100) {
            std::cout << "[" << (line_count + 1) << "] " << token << "\n"; // Display formatted numbered list
            strncpy(path_array[line_count], token, MAX_PATH - 1);
            path_array[line_count][MAX_PATH - 1] = '\0';
            line_count++;
            token = strtok(nullptr, "\n");
        }

        std::cout << "\nEnter the line number to view its content, or 0 to return to main menu: ";
        int file_idx = rse_cli_getUserChoice();

        // User picked a file from the list. Instantly format a completely new DISPLAY command
        // behind the scenes and fire it so the user UX is entirely seamless!
        if (file_idx > 0 && file_idx <= line_count) {
            char display_cmd[MAX_PATH + 32];
            snprintf(display_cmd, sizeof(display_cmd), "%s%s%s", CMD_DISPLAY, CMD_DELIM, path_array[file_idx - 1]);
            rse_cli_sendCommand(sockfd, display_cmd);
            char content_result[BUFFER_SIZE];
            rse_cli_receiveResponse(sockfd, content_result, BUFFER_SIZE);
            std::cout << "\n--- File Content ---\n" << content_result << "\n--------------------\n";
        }
    } else {
        std::cout << "Invalid choice.\n";
    }
}

void rse_cli_handleSearchString(int sockfd) {
    std::cout << "Enter optional base path (default is ./): ";
    char base_path[MAX_PATH];
    std::cin.getline(base_path, MAX_PATH);
    if (strlen(base_path) == 0) {
        strcpy(base_path, "./");
    }

    std::cout << "Enter search query: ";
    char query[MAX_PATH];
    std::cin.getline(query, MAX_PATH);

    char cmd[MAX_PATH * 2 + 32];
    snprintf(cmd, sizeof(cmd), "%s%s%s%s%s", CMD_SEARCH_STR, CMD_DELIM, base_path, CMD_DELIM, query);

    rse_cli_sendCommand(sockfd, cmd);

    char result[BUFFER_SIZE];
    rse_cli_receiveResponse(sockfd, result, BUFFER_SIZE);

    if (strncmp(result, "No matches", 10) == 0 || strncmp(result, "Error", 5) == 0) {
        std::cout << "\n" << result << "\n";
        return;
    }

    std::cout << "\n--- Search Results ---\n";
    
    int line_count = 0;
    char path_array[100][MAX_PATH];
    
    char* token = strtok(result, "\n");
    while (token != nullptr && line_count < 100) {
        std::cout << "[" << (line_count + 1) << "] " << token << "\n";
        
        char token_copy[MAX_PATH + 32];
        strncpy(token_copy, token, sizeof(token_copy) - 1);
        token_copy[sizeof(token_copy) - 1] = '\0';
        
        // Reverse String Search: The server returned "/etc/passwd:5". 
        // We find the last colon and manually slice it using \0 to leave just "/etc/passwd".
        char* last_colon = strrchr(token_copy, ':');
        if (last_colon) {
            *last_colon = '\0'; 
        }
        strncpy(path_array[line_count], token_copy, MAX_PATH - 1);
        path_array[line_count][MAX_PATH - 1] = '\0';
        
        line_count++;
        token = strtok(nullptr, "\n");
    }

    std::cout << "\nEnter file index (1-" << line_count << ") to view contents, or 0 to return to menu: ";
    int choice = rse_cli_getUserChoice();
    if (choice > 0 && choice <= line_count) {
        char display_cmd[MAX_PATH + 32];
        snprintf(display_cmd, sizeof(display_cmd), "%s%s%s", CMD_DISPLAY, CMD_DELIM, path_array[choice - 1]);
        rse_cli_sendCommand(sockfd, display_cmd);
        
        char display_result[BUFFER_SIZE];
        rse_cli_receiveResponse(sockfd, display_result, BUFFER_SIZE);
        std::cout << "\n--- File Contents (" << path_array[choice - 1] << ") ---\n\n";
        std::cout << display_result << "\n";
    }
}

void rse_cli_handleDisplayContent(int sockfd) {
    std::cout << "Enter absolute file path: ";
    char path[MAX_PATH];
    std::cin.getline(path, MAX_PATH);

    char cmd[MAX_PATH + 32];
    snprintf(cmd, sizeof(cmd), "%s%s%s", CMD_DISPLAY, CMD_DELIM, path);

    rse_cli_sendCommand(sockfd, cmd);

    char result[BUFFER_SIZE];
    rse_cli_receiveResponse(sockfd, result, BUFFER_SIZE);
    std::cout << "\n" << result << "\n";
}
