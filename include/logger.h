/*
 * File: logger.h
 * Description: Advanced C-style macro logging utility.
 * Prints strictly formatted timestamps to the terminal AND appends to rse.log.
 * Author: Originator
 */
#ifndef LOGGER_H
#define LOGGER_H

#include <cstdio>
#include <ctime>

// Writes HH:MM:SS into the provided char array buffer.
// Wrapping this in `do { ... } while(0)` forces the macro to behave dynamically
// as a single, safe execution block requiring a trailing semicolon.
#define LOG_TIMESTAMP(p_buf) \
    do { \
        time_t _t = time(nullptr); \
        struct tm* _tm = localtime(&_t); \
        strftime(p_buf, sizeof(p_buf), "%H:%M:%S", _tm); \
    } while (0)

// The Core Engine Macro.
// __VA_OPT__(,) __VA_ARGS__ is a modern preprocessor trick that allows 
// this macro to cleanly accept a variable number of arguments exactly like printf().
#define LOG(level, fmt, ...) \
    do { \
        char _ts[16]; \
        LOG_TIMESTAMP(_ts); \
        /* Print to the Terminal */ \
        printf("[%s][%s] " fmt "\n", _ts, level __VA_OPT__(,) __VA_ARGS__); \
        /* Simultaneously print to the permanent disk log */ \
        FILE* _f = fopen("rse.log", "a"); \
        if (_f) { \
            fprintf(_f, "[%s][%s] " fmt "\n", _ts, level __VA_OPT__(,) __VA_ARGS__); \
            fclose(_f); \
        } \
    } while (0)

// Syntactic Sugar wrappers.
// These save typing by automatically injecting the severity string.
#define LOG_DEBUG(fmt, ...)  LOG("DEBUG", fmt __VA_OPT__(,) __VA_ARGS__)
#define LOG_INFO(fmt, ...)   LOG("INFO ", fmt __VA_OPT__(,) __VA_ARGS__)
#define LOG_WARN(fmt, ...)   LOG("WARN ", fmt __VA_OPT__(,) __VA_ARGS__)
#define LOG_FATAL(fmt, ...)  LOG("FATAL", fmt __VA_OPT__(,) __VA_ARGS__)

// Zero-Argument Fallbacks.
// Defensively created to satisfy strict GCC compilers that might throw warnings
// if __VA_ARGS__ is completely empty.
#define LOG_DEBUG0(msg)  LOG("DEBUG", "%s", msg)
#define LOG_INFO0(msg)   LOG("INFO ", "%s", msg)
#define LOG_WARN0(msg)   LOG("WARN ", "%s", msg)
#define LOG_FATAL0(msg)  LOG("FATAL", "%s", msg)

#endif // LOGGER_H
