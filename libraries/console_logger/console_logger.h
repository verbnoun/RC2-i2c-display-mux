#ifndef CONSOLE_LOGGER_H
#define CONSOLE_LOGGER_H

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "pico/stdlib.h"

// Log levels
typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO  = 1,
    LOG_LEVEL_WARN  = 2,
    LOG_LEVEL_ERROR = 3,
    LOG_LEVEL_NONE  = 4
} log_level_t;

// Color codes for console output
#define COLOR_RESET   "\033[0m"
#define COLOR_BLACK   "\033[30m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"

// Bright colors
#define COLOR_BRIGHT_BLACK   "\033[90m"
#define COLOR_BRIGHT_RED     "\033[91m"
#define COLOR_BRIGHT_GREEN   "\033[92m"
#define COLOR_BRIGHT_YELLOW  "\033[93m"
#define COLOR_BRIGHT_BLUE    "\033[94m"
#define COLOR_BRIGHT_MAGENTA "\033[95m"
#define COLOR_BRIGHT_CYAN    "\033[96m"
#define COLOR_BRIGHT_WHITE   "\033[97m"

// Common tags for standardization across frameworks
#define TAG_CONTROLLER  "CTRL"
#define TAG_SYNTH      "SYNTH"
#define TAG_HW         "HW"
#define TAG_MIDI       "MIDI"
#define TAG_AUDIO      "AUDIO"
#define TAG_OLED       "OLED"
#define TAG_POT        "POT"
#define TAG_MUX        "MUX"
#define TAG_I2C        "I2C"
#define TAG_USB        "USB"
#define TAG_ANIM       "ANIM"
#define TAG_BOOT       "BOOT"
#define TAG_SYSTEM     "SYS"
#define TAG_NORM       "NORM"

// Maximum tag length and message length
#define MAX_TAG_LENGTH 8
#define MAX_MESSAGE_LENGTH 256

class ConsoleLogger {
public:
    /**
     * Initialize the console logger
     * @param level Minimum log level to output
     * @param enable_colors Enable colored output (default: true)
     * @param enable_timestamps Enable timestamps (default: false, saves memory)
     */
    static void init(log_level_t level = LOG_LEVEL_INFO, bool enable_colors = true, bool enable_timestamps = false);
    
    /**
     * Set the current log level
     */
    static void setLogLevel(log_level_t level);
    
    /**
     * Get the current log level
     */
    static log_level_t getLogLevel();
    
    /**
     * Log a message with specified level and tag
     */
    static void log(log_level_t level, const char* tag, const char* format, ...);
    
    /**
     * Convenience functions for each log level
     */
    static void debug(const char* tag, const char* format, ...);
    static void info(const char* tag, const char* format, ...);
    static void warn(const char* tag, const char* format, ...);
    static void error(const char* tag, const char* format, ...);
    
    /**
     * Raw printf-style output without formatting (for special cases)
     */
    static void raw(const char* format, ...);
    
    /**
     * Print a banner/header message
     */
    static void banner(const char* message);
    
    /**
     * Print system info (memory, uptime, etc.)
     */
    static void systemInfo();
    
    /**
     * Tag enable/disable functions
     */
    static void enableTag(const char* tag);
    static void disableTag(const char* tag);
    static void toggleTag(const char* tag);
    static bool isTagEnabled(const char* tag);
    static void showEnabledTags();

private:
    static log_level_t current_level_;
    static bool colors_enabled_;
    static bool timestamps_enabled_;
    static uint32_t enabled_tags_;
    
    static const char* getLevelString(log_level_t level);
    static const char* getLevelColor(log_level_t level);
    static void formatTimestamp(char* buffer, size_t buffer_size);
    static void vlog(log_level_t level, const char* tag, const char* format, va_list args);
    static uint8_t tagToBit(const char* tag);
};

// Tag-based logging macros (ignores log level, uses tag enable/disable)
#define LOG(tag, ...) ConsoleLogger::log(LOG_LEVEL_INFO, tag, __VA_ARGS__)

// Legacy convenience macros (still useful for always-on logging)
#define LOG_DEBUG(tag, ...) ConsoleLogger::debug(tag, __VA_ARGS__)
#define LOG_INFO(tag, ...)  ConsoleLogger::info(tag, __VA_ARGS__)
#define LOG_WARN(tag, ...)  ConsoleLogger::warn(tag, __VA_ARGS__)
#define LOG_ERROR(tag, ...) ConsoleLogger::error(tag, __VA_ARGS__)

// Quick macros for common tags
#define LOG_HW_DEBUG(...)   LOG_DEBUG(TAG_HW, __VA_ARGS__)
#define LOG_HW_INFO(...)    LOG_INFO(TAG_HW, __VA_ARGS__)
#define LOG_HW_WARN(...)    LOG_WARN(TAG_HW, __VA_ARGS__)
#define LOG_HW_ERROR(...)   LOG_ERROR(TAG_HW, __VA_ARGS__)

#define LOG_BOOT_INFO(...)  LOG_INFO(TAG_BOOT, __VA_ARGS__)
#define LOG_BOOT_WARN(...)  LOG_WARN(TAG_BOOT, __VA_ARGS__)
#define LOG_BOOT_ERROR(...) LOG_ERROR(TAG_BOOT, __VA_ARGS__)

#endif // CONSOLE_LOGGER_H