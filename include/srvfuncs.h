/*
 * File: srvfuncs.h
 * Description: The Server Interface ("Table of Contents").
 * Exposes the heavy-lifting logic modules so server.cpp can call them 
 * without compiling their source directly into itself.
 * Author: Originator
 */
#ifndef SRVFUNCS_H
#define SRVFUNCS_H

// ============================================
// Architecture Guidelines:
// - `rse_srv_`: Strict naming format (<stack>_<module>_<name>)
// - `p_`: Pointer prefix demanding careful memory handling
// - `p_result`: The 64KB network buffer passed in to store the answer
// ============================================

// Option 1: Validates if a file exists and is a regular readable file.
// Takes absolute path p_path, writes status into p_result.
void rse_srv_searchForFile(const char* p_path, char* p_result);

// Option 2 (Helper): Recursively searches starting at base_path for a specific filename. 
void rse_srv_findFile(const char* p_base_path, const char* p_filename, char* p_result);

// Option 2: The deep recursive string search engine. (Entry Point)
// Scopes the search to p_base_path, looking for text matching p_query.
void rse_srv_searchForString(const char* p_base_path, const char* p_query, char* p_result);

// Option 2 (Walker): Recursively walks a directory tree, skipping virtual OS landmines.
void rse_srv_searchInDirectory(const char* p_dir, const char* p_query, char* p_result);

// Option 2 (Line Scanner): Line-by-Line case-insensitive strcasestr lookup.
void rse_srv_searchInFile(const char* p_filepath, const char* p_query, char* p_result);

// Option 3: Dumps up to 64KB of a raw file into memory.
void rse_srv_displayFileContent(const char* p_filepath, char* p_result);

// Legacy/Deprecated: Un-refactored leftover function from before the AAPE guidelines.
void displayFileContent(const char* path, char* result);

#endif // SRVFUNCS_H
