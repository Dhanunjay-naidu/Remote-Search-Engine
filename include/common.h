/*
 * File: common.h
 * Description: The Core "Data Contract" for RSE.
 * Both the Client and Server include this file to guarantee they 
 * use the exact same network port, buffer limits, and string vocabulary.
 * Author: Originator
 */
#ifndef COMMON_H // Include Guards: Prevent compiler from crashing if included twice
#define COMMON_H

// ============================================
// Network Settings
// ============================================
#define PORT        5678 // The fixed TCP port the server binds to
#define BACKLOG     5    // Max number of clients that can wait in the OS queue if the server is busy

// ============================================
// Memory & Buffer Limits
// ============================================
// BUFFER_SIZE: The "Bucket Size". Exactly 64 Kilobytes. 
// Standardized here so neither client nor server accidentally overflows the other.
#define BUFFER_SIZE 65536 
#define MAX_PATH    1024  // Maximum safe string length for a Linux filepath

// ============================================
// Protocol Vocabulary
// ============================================
// These are Preprocessor Macros. The compiler safely replaces them with strings.
// Using macros prevents invisible typos (e.g. spelling "SEACH_FILE" directly in code).
#define CMD_SEARCH_FILE  "SEARCH_FILE"
#define CMD_FIND_FILE    "FIND_FILE"
#define CMD_SEARCH_STR   "SEARCH_STR"
#define CMD_DISPLAY      "DISPLAY"
#define CMD_EXIT         "EXIT"

// The exact character used in the text payload to separate commands from arguments.
// Example over TCP: "SEARCH_STR:./:root"
#define CMD_DELIM        ":"

#endif // COMMON_H
