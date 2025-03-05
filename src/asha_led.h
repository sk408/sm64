/**
 * @file asha_led.h
 * @brief LED indicator module for Pico-ASHA
 */

#ifndef ASHA_LED_H
#define ASHA_LED_H

#include <stdint.h>

/**
 * @brief LED patterns
 */
typedef enum {
    LED_PATTERN_OFF,              ///< LED off
    LED_PATTERN_ON,               ///< LED on
    LED_PATTERN_BLINK_SLOW,       ///< Slow blinking (1 Hz)
    LED_PATTERN_BLINK_FAST,       ///< Fast blinking (5 Hz)
    LED_PATTERN_PULSE,            ///< Pulse (fade in/out)
    LED_PATTERN_DOUBLE_BLINK,     ///< Double blink (two quick blinks, then pause)
    LED_PATTERN_TRIPLE_BLINK,     ///< Triple blink (three quick blinks, then pause)
    LED_PATTERN_SOS              ///< SOS pattern (... --- ...)
} led_pattern_t;

/**
 * @brief Initialize the LED module
 * 
 * @return 0 on success, non-zero on failure
 */
int led_init(void);

/**
 * @brief Set the LED pattern
 * 
 * @param pattern The pattern to set
 */
void led_set_pattern(led_pattern_t pattern);

/**
 * @brief Get the current LED pattern
 * 
 * @return The current LED pattern
 */
led_pattern_t led_get_pattern(void);

/**
 * @brief Process the LED state (call this in a loop)
 * 
 * @param ms_elapsed Milliseconds elapsed since last call
 */
void led_process(uint32_t ms_elapsed);

/**
 * @brief Set the LED brightness (0-255)
 * 
 * @param brightness The brightness to set (0-255)
 */
void led_set_brightness(uint8_t brightness);

/**
 * @brief Get the current LED brightness
 * 
 * @return The current LED brightness (0-255)
 */
uint8_t led_get_brightness(void);

/**
 * @brief Set the LED on or off (convenience function)
 * 
 * @param on 1 to turn on, 0 to turn off
 */
void led_set_on(uint8_t on);

#endif /* ASHA_LED_H */ 