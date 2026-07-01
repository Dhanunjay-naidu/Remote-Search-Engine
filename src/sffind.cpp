/*
 * File: sffind.cpp
 * Description: Recursively searches starting at base path for filenames matching target.
 * Utilizes modern C++17 <filesystem> for graceful error handling.
 * Author: Originator
 */
#include <cstring>
#include <cstdio>
#include <filesystem>
#include <string>

#include "common.h"
#include "logger.h"
#include "srvfuncs.h"

// Scans an entire directory tree for a file named p_target_filename.
void rse_srv_findFile(const char* p_base_path, const char* p_target_filename, char* p_result) {
    LOG_DEBUG("findFile: %s in %s", p_target_filename, p_base_path);
    namespace fs = std::filesystem;

    // Critical Memory Hygiene: Wipe leftover data from previous client's search
    memset(p_result, 0, BUFFER_SIZE);
    bool found = false;
    std::string current_dir = p_base_path;

    try {
        // C++17 Magic: Iterates deeply through all subfolders automatically.
        // skip_permission_denied is essential here: it silently ignores admin-locked folders 
        // (like /root) instead of throwing a fatal terminal C++ exception and crashing the server.
        for (const auto& entry : fs::recursive_directory_iterator(current_dir, fs::directory_options::skip_permission_denied)) {
            
            // Validate it is an actual physical file, and the string matches
            if (entry.is_regular_file() && entry.path().filename() == p_target_filename) {
                found = true;
                
                // Temporarily store the absolute path with a newline
                char line[MAX_PATH + 10];
                snprintf(line, sizeof(line), "%s\n", entry.path().c_str()); 
                
                // Buffer Overflow Defense:
                // Only append to the massive string if there is mathematical room left in the 64KB limit.
                if (strlen(p_result) + strlen(line) < BUFFER_SIZE - 1) {
                    strcat(p_result, line);
                } else {
                    LOG_WARN0("Result buffer overflow during findFile. Truncating results.");
                    break; // Escape the loop early, preventing RAM corruption
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        LOG_WARN("Cannot access directory: %s", p_base_path);
    }

    if (!found) {
        LOG_INFO("Files not found matching filename: %s", p_target_filename);
        snprintf(p_result, BUFFER_SIZE, "(no matches)\n");
    } else {
        LOG_INFO("Files found matching filename: %s", p_target_filename);
    }
}
