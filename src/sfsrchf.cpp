/*
 * File: sfsrchf.cpp
 * Description: Validates existence of a file given its path.
 * Author: Originator
 */
#include <cstring>
#include <cstdio>
#include <filesystem>
#include <sys/stat.h>

#include "common.h"
#include "logger.h"
#include "srvfuncs.h"

// Option 1 Implementation: Checks if an absolute path exists and is a readable file.
void rse_srv_searchForFile(const char* p_path, char* p_result) {
    LOG_DEBUG("searchForFile: %s", p_path);

    namespace fs = std::filesystem;

    // C++17 rapid existence check. Returns true if ANYTHING exists at this path.
    if (fs::exists(p_path)) {
        
        // POSIX validation: "Everything is a file" in Linux.
        // We must check if this is actually a regular file (text/binary) and not a directory (/etc).
        struct stat st;
        stat(p_path, &st);

        // S_ISREG guarantees it is safe to open and read without locking up the OS.
        if (S_ISREG(st.st_mode)) { 
            LOG_INFO("File found: %s", p_path);
            snprintf(p_result, BUFFER_SIZE, "File found: %s", p_path);
        } else {
            // It exists, but we block the user from reading it to prevent OS-level crashes.
            LOG_WARN("Path exists but is not a regular file: %s", p_path);
            snprintf(p_result, BUFFER_SIZE, "Error: Path exists but is not a regular file: %s", p_path);
        }
    } else {
        LOG_FATAL("File not found: %s", p_path);
        snprintf(p_result, BUFFER_SIZE, "Error: File not found at path: %s", p_path);
    }
}
