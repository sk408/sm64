/**
 * @file asha_logging.h
 * @brief Logging system for Pico-ASHA
 */

#ifndef ASHA_LOGGING_H
#define ASHA_LOGGING_H

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

/**
 * @brief Log levels for the logging system
 */
typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_NONE
} log_level_t;

/**
 * @brief Initialize the logging system
 * 
 * @param level The minimum log level to display
 * @return 0 on success, non-zero on failure
 */
int logging_init(log_level_t level);

/**
 * @brief Set the logging level
 * 
 * @param level The new minimum log level to display
 */
void logging_set_level(log_level_t level);

/**
 * @brief Get the current logging level
 * 
 * @return The current minimum log level
 */
log_level_t logging_get_level(void);

/**
 * @brief Log a message at the specified level
 * 
 * @param level The log level for this message
 * @param format The format string for the message
 * @param ... The variable arguments for the format string
 */
void log_message(log_level_t level, const char *format, ...);

/**
 * @brief Log a message at debug level
 * 
 * @param format The format string for the message
 * @param ... The variable arguments for the format string
 */
#define LOG_DEBUG(format, ...) log_message(LOG_LEVEL_DEBUG, format, ##__VA_ARGS__)

/**
 * @brief Log a message at info level
 * 
 * @param format The format string for the message
 * @param ... The variable arguments for the format string
 */
#define LOG_INFO(format, ...) log_message(LOG_LEVEL_INFO, format, ##__VA_ARGS__)

/**
 * @brief Log a message at warning level
 * 
 * @param format The format string for the message
 * @param ... The variable arguments for the format string
 */
#define LOG_WARNING(format, ...) log_message(LOG_LEVEL_WARNING, format, ##__VA_ARGS__)

/**
 * @brief Log a message at error level
 * 
 * @param format The format string for the message
 * @param ... The variable arguments for the format string
 */
#define LOG_ERROR(format, ...) log_message(LOG_LEVEL_ERROR, format, ##__VA_ARGS__)

/**
 * @brief Get the number of log messages in the buffer
 * 
 * @return The number of log messages in the buffer
 */
uint32_t logging_get_count(void);

/**
 * @brief Get a log message from the buffer
 * 
 * @param index The index of the log message to get
 * @param buffer The buffer to store the log message
 * @param buffer_size The size of the buffer
 * @return The number of bytes written to the buffer, or -1 on error
 */
int logging_get_message(uint32_t index, char *buffer, size_t buffer_size);

/**
 * @brief Clear the log buffer
 */
void logging_clear(void);

#endif /* ASHA_LOGGING_H */ 