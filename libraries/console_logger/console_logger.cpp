#include "console_logger.h"
#include "pico/time.h"
#include "pico/unique_id.h"
#include "hardware/watchdog.h"

// Static member definitions
log_level_t ConsoleLogger::current_level_ = LOG_LEVEL_INFO;
bool ConsoleLogger::colors_enabled_ = true;
bool ConsoleLogger::timestamps_enabled_ = false;

void ConsoleLogger::init(log_level_t level, bool enable_colors, bool enable_timestamps) {
    current_level_ = level;
    colors_enabled_ = enable_colors;
    timestamps_enabled_ = enable_timestamps;
    
    // Ensure stdio is initialized
    stdio_init_all();
    
    // Small delay for terminal to be ready
    sleep_ms(100);
}

void ConsoleLogger::setLogLevel(log_level_t level) {
    current_level_ = level;
}

log_level_t ConsoleLogger::getLogLevel() {
    return current_level_;
}

void ConsoleLogger::log(log_level_t level, const char* tag, const char* format, ...) {
    if (level < current_level_) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    vlog(level, tag, format, args);
    va_end(args);
}

void ConsoleLogger::debug(const char* tag, const char* format, ...) {
    if (LOG_LEVEL_DEBUG < current_level_) return;
    
    va_list args;
    va_start(args, format);
    vlog(LOG_LEVEL_DEBUG, tag, format, args);
    va_end(args);
}

void ConsoleLogger::info(const char* tag, const char* format, ...) {
    if (LOG_LEVEL_INFO < current_level_) return;
    
    va_list args;
    va_start(args, format);
    vlog(LOG_LEVEL_INFO, tag, format, args);
    va_end(args);
}

void ConsoleLogger::warn(const char* tag, const char* format, ...) {
    if (LOG_LEVEL_WARN < current_level_) return;
    
    va_list args;
    va_start(args, format);
    vlog(LOG_LEVEL_WARN, tag, format, args);
    va_end(args);
}

void ConsoleLogger::error(const char* tag, const char* format, ...) {
    if (LOG_LEVEL_ERROR < current_level_) return;
    
    va_list args;
    va_start(args, format);
    vlog(LOG_LEVEL_ERROR, tag, format, args);
    va_end(args);
}

void ConsoleLogger::raw(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fflush(stdout);
}

void ConsoleLogger::banner(const char* message) {
    const int banner_width = 60;
    const char border_char = '=';
    
    if (colors_enabled_) {
        printf("%s", COLOR_BRIGHT_CYAN);
    }
    
    // Top border
    for (int i = 0; i < banner_width; i++) {
        printf("%c", border_char);
    }
    printf("\n");
    
    // Message line with padding
    int message_len = strlen(message);
    int padding = (banner_width - 4 - message_len) / 2; // 4 for "| " and " |"
    
    printf("%c%c", border_char, border_char);
    for (int i = 0; i < padding; i++) printf(" ");
    printf("%s", message);
    for (int i = 0; i < (banner_width - 4 - message_len - padding); i++) printf(" ");
    printf("%c%c\n", border_char, border_char);
    
    // Bottom border
    for (int i = 0; i < banner_width; i++) {
        printf("%c", border_char);
    }
    printf("\n");
    
    if (colors_enabled_) {
        printf("%s", COLOR_RESET);
    }
    
    fflush(stdout);
}

void ConsoleLogger::systemInfo() {
    // Get unique board ID
    pico_unique_board_id_t board_id;
    pico_get_unique_board_id(&board_id);
    
    // Get uptime
    uint32_t uptime_ms = to_ms_since_boot(get_absolute_time());
    
    info(TAG_SYSTEM, "Board ID: %02x%02x%02x%02x%02x%02x%02x%02x",
         board_id.id[0], board_id.id[1], board_id.id[2], board_id.id[3],
         board_id.id[4], board_id.id[5], board_id.id[6], board_id.id[7]);
    
    info(TAG_SYSTEM, "Uptime: %lu ms (%.2f seconds)", uptime_ms, uptime_ms / 1000.0f);
    
    // Check if watchdog is running
    if (watchdog_caused_reboot()) {
        warn(TAG_SYSTEM, "Previous reboot caused by watchdog timeout");
    }
}

const char* ConsoleLogger::getLevelString(log_level_t level) {
    switch (level) {
        case LOG_LEVEL_DEBUG: return "DEBUG";
        case LOG_LEVEL_INFO:  return "INFO ";
        case LOG_LEVEL_WARN:  return "WARN ";
        case LOG_LEVEL_ERROR: return "ERROR";
        default:              return "?????";
    }
}

const char* ConsoleLogger::getLevelColor(log_level_t level) {
    if (!colors_enabled_) return "";
    
    switch (level) {
        case LOG_LEVEL_DEBUG: return COLOR_BRIGHT_BLACK;
        case LOG_LEVEL_INFO:  return COLOR_BRIGHT_BLUE;
        case LOG_LEVEL_WARN:  return COLOR_BRIGHT_YELLOW;
        case LOG_LEVEL_ERROR: return COLOR_BRIGHT_RED;
        default:              return COLOR_WHITE;
    }
}

void ConsoleLogger::formatTimestamp(char* buffer, size_t buffer_size) {
    if (!timestamps_enabled_) {
        buffer[0] = '\0';
        return;
    }
    
    uint32_t uptime_ms = to_ms_since_boot(get_absolute_time());
    uint32_t seconds = uptime_ms / 1000;
    uint32_t ms = uptime_ms % 1000;
    
    snprintf(buffer, buffer_size, "[%3lu.%03lu] ", seconds, ms);
}

void ConsoleLogger::vlog(log_level_t level, const char* tag, const char* format, va_list args) {
    char timestamp_buffer[16];
    char message_buffer[MAX_MESSAGE_LENGTH];
    char tag_buffer[MAX_TAG_LENGTH + 1];
    
    // Format timestamp if enabled
    formatTimestamp(timestamp_buffer, sizeof(timestamp_buffer));
    
    // Ensure tag is properly sized
    snprintf(tag_buffer, sizeof(tag_buffer), "%-*s", MAX_TAG_LENGTH - 1, tag ? tag : "NULL");
    
    // Format the message
    vsnprintf(message_buffer, sizeof(message_buffer), format, args);
    
    // Output the complete log line
    if (colors_enabled_) {
        printf("%s%s[%s%s%s] %s%s%s\n",
               timestamp_buffer,
               getLevelColor(level),
               getLevelString(level),
               COLOR_RESET,
               COLOR_BRIGHT_WHITE,
               tag_buffer,
               COLOR_RESET,
               message_buffer);
    } else {
        printf("%s[%s] %s %s\n",
               timestamp_buffer,
               getLevelString(level),
               tag_buffer,
               message_buffer);
    }
    
    fflush(stdout);
}