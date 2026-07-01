#include <cstring>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>

/*
 * File: sfsrchs.cpp
 * Description: Recursively searches text files for a query string.
 * This is the most complex module, broken into Single Responsibility components:
 * 1. Entry Point
 * 2. Directory Walker (Recursion)
 * 3. Line Scanner
 * Author: Originator
 */
#include "common.h"
#include "logger.h"
#include "srvfuncs.h"

// ==========================================
// 3. The Line Scanner
// ==========================================
// Opens one specific file, reads it line by line, and checks for the string.
void rse_srv_searchInFile(const char* p_filepath, const char* p_query, char* p_result) {
    FILE* fp = fopen(p_filepath, "r");
    if (!fp) {
        LOG_WARN("Cannot open file: %s", p_filepath);
        return;
    }

    char line[1024];
    int lineNum = 0;

    // Use fgets to read chunks at a time. This prevents a massive 10GB log file 
    // from being loaded into the server's RAM all at once.
    while (fgets(line, sizeof(line), fp)) {
        lineNum++;
        
        // strcasestr is a GNU extension for case-insensitive lookup (e.g. matches "ROOT" to "root")
        if (strcasestr(line, p_query)) {
            LOG_INFO("Match in %s at line %d", p_filepath, lineNum);
            
            char entry[MAX_PATH + 32];
            snprintf(entry, sizeof(entry), "%s:%d\n", p_filepath, lineNum);
            
            // Critical Memory Security: Strict bound checking before appending.
            // If the user's query triggers 5,000 matches, this stops the buffer from overflowing 64KB.
            if (strlen(p_result) + strlen(entry) < (size_t)(BUFFER_SIZE - 1))
                strncat(p_result, entry, BUFFER_SIZE - strlen(p_result) - 1);
        }
    }

    fclose(fp);
}

// ==========================================
// 2. The Directory Walker
// ==========================================
// Opens a folder and loops through its contents. If it finds a subfolder, it calls itself.
void rse_srv_searchInDirectory(const char* p_dir, const char* p_query, char* p_result) {
    DIR* dp = opendir(p_dir);
    if (!dp) {
        LOG_WARN("Cannot open directory: %s", p_dir);
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dp)) != nullptr) {
        // Infinite Loop Prevention: Skip self (.) and parent (..) references
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char fullpath[MAX_PATH];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", p_dir, entry->d_name);

        // System Defenses: Skip virtual kernel directories. 
        // /dev/urandom will generate infinite bytes if opened, crashing the server.
        if (strstr(fullpath, "/proc") || strstr(fullpath, "/sys") || strstr(fullpath, "/dev"))
            continue;

        struct stat st;
        if (stat(fullpath, &st) != 0)
            continue;

        if (S_ISREG(st.st_mode)) {
            rse_srv_searchInFile(fullpath, p_query, p_result); // It's a file, scan it
        } else if (S_ISDIR(st.st_mode)) {
            rse_srv_searchInDirectory(fullpath, p_query, p_result); // It's a folder, RECURSE deeper
        }
    }

    closedir(dp);
}

// ==========================================
// 1. The Entry Point
// ==========================================
void rse_srv_searchForString(const char* p_base_path, const char* p_query, char* p_result) {
    LOG_INFO("Starting string search for: %s in %s", p_query, p_base_path);

    // O(1) Time Complexity optimization: Dropping a null-terminator at index 0 
    // instantly "clears" a C-string without requiring an O(N) memset over 64,000 bytes.
    p_result[0] = '\0'; 
    rse_srv_searchInDirectory(p_base_path, p_query, p_result);

    // If the recursive walker finishes and the buffer is still empty, nothing was found.
    if (strlen(p_result) == 0) {
        snprintf(p_result, BUFFER_SIZE, "No matches found for: %s", p_query);
        LOG_WARN("No matches found for query: %s", p_query);
    }
}
