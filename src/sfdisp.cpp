/*
 * File: sfdisp.cpp
 * Description: Reads file content and returns it.
 * Author: Originator
 */
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <filesystem>

#include "common.h"
#include "logger.h"
#include "srvfuncs.h"

// Option 3 Implementation: Dumps a file directly into the 64KB network buffer
void rse_srv_displayFileContent(const char* p_filepath, char* p_result) {
    LOG_DEBUG("displayFileContent: %s", p_filepath);

    // Hygiene: Clear any leftover data from a previous client's request
    memset(p_result, 0, BUFFER_SIZE);

    // Rapid C++17 check to gracefully abort before attempting disk I/O
    if (!std::filesystem::exists(p_filepath)) {
        LOG_FATAL("Cannot open file: %s", p_filepath);
        snprintf(p_result, BUFFER_SIZE, "Error: Cannot open file: %s", p_filepath);
        return;
    }

    // Classic C FILE stream in Read-Only ("r") mode
    FILE* file = fopen(p_filepath, "r");
    if (!file) {
        LOG_FATAL("Cannot open file: %s", p_filepath);
        snprintf(p_result, BUFFER_SIZE, "Error: Cannot open file: %s", p_filepath);
        return;
    }

    // High Performance I/O: 
    // Instead of looping via fgets, `fread` reads a massive raw binary chunk from the hard drive 
    // and copies it directly into RAM in one action. Capped strictly to BUFFER_SIZE - 1.
    size_t bytesRead = fread(p_result, 1, BUFFER_SIZE - 1, file);
    
    // Safety: Because fread reads raw binary, it does NOT insert a string null-terminator.
    // We mathematically calculate the end of the text and manually cap it so it acts like a safe C-string.
    p_result[bytesRead] = '\0';
    fclose(file);

    LOG_INFO("File content sent for: %s", p_filepath);
}
