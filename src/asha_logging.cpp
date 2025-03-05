/**
 * @file asha_logging.cpp
 * @brief Logging system implementation for Pico-ASHA
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <pico/stdlib.h>
#include "asha_logging.h"

#define LOG_BUFFER_SIZE 32  // Number of log messages to store
#define LOG_MESSAGE_SIZE 128 // Maximum size of a log message

// Circular buffer for log messages
typedef struct {
    char messages[LOG_BUFFER_SIZE][LOG_MESSAGE_SIZE];
    uint32_t write_index;
    uint32_t count;
} log_buffer_t;

// Log level strings
static const char *log_level_strings[] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
    "NONE"
};

// Current log level
static log_level_t current_level = LOG_LEVEL_INFO;

// Log buffer
static log_buffer_t log_buffer;

// Initialize the logging system
int logging_init(log_level_t level) {
    current_level = level;
    memset(&log_buffer, 0, sizeof(log_buffer));
    return 0;
}

// Set the logging level
void logging_set_level(log_level_t level) {
    current_level = level;
}

// Get the current logging level
log_level_t logging_get_level(void) {
    return current_level;
}

// Log a message at the specified level
void log_message(log_level_t level, const char *format, ...) {
    // Check if the message should be logged
    if (level < current_level) {
        return;
    }
    
    // Get the current time
    absolute_time_t now = get_absolute_time();
    uint32_t ms = to_ms_since_boot(now);
    uint32_t seconds = ms / 1000;
    ms %= 1000;
    
    // Format the timestamp
    char timestamp[16];
    snprintf(timestamp, sizeof(timestamp), "%lu.%03lu", seconds, ms);
    
    // Format the log level
    const char *level_str = log_level_strings[level];
    
    // Format the message
    va_list args;
    va_start(args, format);
    
    // Calculate the index to write to
    uint32_t index = log_buffer.write_index;
    
    // Format the full message with timestamp and level
    snprintf(log_buffer.messages[index], LOG_MESSAGE_SIZE, "%s [%s] ", timestamp, level_str);
    
    // Append the actual message
    size_t prefix_len = strlen(log_buffer.messages[index]);
    vsnprintf(log_buffer.messages[index] + prefix_len, LOG_MESSAGE_SIZE - prefix_len, format, args);
    
    va_end(args);
    
    // Output to stdio (USB)
    printf("%s\n", log_buffer.messages[index]);
    
    // Update the write index
    log_buffer.write_index = (log_buffer.write_index + 1) % LOG_BUFFER_SIZE;
    
    // Update the count
    if (log_buffer.count < LOG_BUFFER_SIZE) {
        log_buffer.count++;
    }
}

// Get the number of log messages in the buffer
uint32_t logging_get_count(void) {
    return log_buffer.count;
}

// Get a log message from the buffer
int logging_get_message(uint32_t index, char *buffer, size_t buffer_size) {
    // Check if the index is valid
    if (index >= log_buffer.count) {
        return -1;
    }
    
    // Calculate the actual index in the circular buffer
    uint32_t actual_index;
    if (log_buffer.count < LOG_BUFFER_SIZE) {
        actual_index = index;
    } else {
        actual_index = (log_buffer.write_index + index) % LOG_BUFFER_SIZE;
    }
    
    // Copy the message to the buffer
    size_t len = strlen(log_buffer.messages[actual_index]);
    size_t copy_len = (len < buffer_size - 1) ? len : (buffer_size - 1);
    memcpy(buffer, log_buffer.messages[actual_index], copy_len);
    buffer[copy_len] = '\0';
    
    return copy_len;
}

// Clear the log buffer
void logging_clear(void) {
    memset(&log_buffer, 0, sizeof(log_buffer));
} 