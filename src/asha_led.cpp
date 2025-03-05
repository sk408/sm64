/**
 * @file asha_led.cpp
 * @brief LED indicator implementation for Pico-ASHA
 */

#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/pwm.h>
#include <hardware/gpio.h>
#include "asha_led.h"
#include "asha_logging.h"

// Use the built-in LED (GPIO 25 on Pico, CYW43 on Pico W)
#if defined(PICO_DEFAULT_LED_PIN)
    #define LED_PIN PICO_DEFAULT_LED_PIN
#else
    #define LED_PIN 25
#endif

// PWM configuration
#define PWM_WRAP 255
#define PWM_CLOCK_DIV 125  // 125MHz / 125 = 1MHz

// Pattern timing (in ms)
#define BLINK_SLOW_PERIOD 1000
#define BLINK_FAST_PERIOD 200
#define PULSE_PERIOD 2000
#define DOUBLE_BLINK_ON_TIME 100
#define DOUBLE_BLINK_OFF_TIME 100
#define DOUBLE_BLINK_PAUSE_TIME 800
#define TRIPLE_BLINK_ON_TIME 100
#define TRIPLE_BLINK_OFF_TIME 100
#define TRIPLE_BLINK_PAUSE_TIME 800
#define SOS_DOT_TIME 200
#define SOS_DASH_TIME 600
#define SOS_ELEMENT_PAUSE 200
#define SOS_LETTER_PAUSE 600
#define SOS_WORD_PAUSE 1400

// Current state
static led_pattern_t current_pattern = LED_PATTERN_OFF;
static uint8_t current_brightness = 255;
static uint32_t pattern_time = 0;
static uint8_t pattern_state = 0;
static uint16_t pwm_slice_num;

// SOS pattern: 3 dots, 3 dashes, 3 dots
static const uint8_t sos_pattern[] = {
    1, 0, 1, 0, 1, 0,  // Three dots
    2, 0, 2, 0, 2, 0,  // Three dashes
    1, 0, 1, 0, 1, 0,  // Three dots
    3                  // Word pause
};

// Function to set the physical LED brightness (0-255)
static void set_led_pwm(uint8_t value) {
#if defined(CYW43_USES_CORE1_FOR_BLUETOOTH)
    // On Pico W with Bluetooth, use the CYW43 API to control the LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, value > 0);
#else
    // Set PWM duty cycle based on brightness and value
    uint16_t pwm_value = (value * current_brightness) / 255;
    pwm_set_chan_level(pwm_slice_num, PWM_CHAN_A, pwm_value);
#endif
}

int led_init(void) {
    LOG_INFO("Initializing LED module");
    
#if defined(CYW43_USES_CORE1_FOR_BLUETOOTH)
    // On Pico W with Bluetooth, the LED is handled by the CYW43 driver
    // No additional initialization needed
#else
    // Configure the LED pin for PWM
    gpio_set_function(LED_PIN, GPIO_FUNC_PWM);
    
    // Configure PWM
    pwm_slice_num = pwm_gpio_to_slice_num(LED_PIN);
    pwm_set_wrap(pwm_slice_num, PWM_WRAP);
    pwm_set_clkdiv(pwm_slice_num, PWM_CLOCK_DIV);
    pwm_set_chan_level(pwm_slice_num, PWM_CHAN_A, 0);
    pwm_set_enabled(pwm_slice_num, true);
#endif
    
    // Start with LED off
    set_led_pwm(0);
    
    LOG_DEBUG("LED initialized on GPIO %d", LED_PIN);
    
    return 0;
}

void led_set_pattern(led_pattern_t pattern) {
    if (pattern != current_pattern) {
        LOG_DEBUG("Setting LED pattern to %d", pattern);
        current_pattern = pattern;
        pattern_time = 0;
        pattern_state = 0;
        
        // Apply the initial pattern state
        if (pattern == LED_PATTERN_ON) {
            set_led_pwm(255);
        } else if (pattern == LED_PATTERN_OFF) {
            set_led_pwm(0);
        }
    }
}

led_pattern_t led_get_pattern(void) {
    return current_pattern;
}

void led_process(uint32_t ms_elapsed) {
    pattern_time += ms_elapsed;
    
    switch (current_pattern) {
        case LED_PATTERN_OFF:
            set_led_pwm(0);
            break;
            
        case LED_PATTERN_ON:
            set_led_pwm(255);
            break;
            
        case LED_PATTERN_BLINK_SLOW:
            if (pattern_time < BLINK_SLOW_PERIOD / 2) {
                set_led_pwm(255);
            } else if (pattern_time < BLINK_SLOW_PERIOD) {
                set_led_pwm(0);
            } else {
                pattern_time = 0;
            }
            break;
            
        case LED_PATTERN_BLINK_FAST:
            if (pattern_time < BLINK_FAST_PERIOD / 2) {
                set_led_pwm(255);
            } else if (pattern_time < BLINK_FAST_PERIOD) {
                set_led_pwm(0);
            } else {
                pattern_time = 0;
            }
            break;
            
        case LED_PATTERN_PULSE:
            {
                // Pulse using a sine wave approximation
                uint8_t brightness;
                if (pattern_time < PULSE_PERIOD / 2) {
                    // Fade in (0 to 255)
                    brightness = (pattern_time * 255) / (PULSE_PERIOD / 2);
                } else if (pattern_time < PULSE_PERIOD) {
                    // Fade out (255 to 0)
                    brightness = 255 - ((pattern_time - PULSE_PERIOD / 2) * 255) / (PULSE_PERIOD / 2);
                } else {
                    pattern_time = 0;
                    brightness = 0;
                }
                set_led_pwm(brightness);
            }
            break;
            
        case LED_PATTERN_DOUBLE_BLINK:
            {
                uint32_t cycle_time = 2 * (DOUBLE_BLINK_ON_TIME + DOUBLE_BLINK_OFF_TIME) + DOUBLE_BLINK_PAUSE_TIME;
                
                if (pattern_time < DOUBLE_BLINK_ON_TIME) {
                    set_led_pwm(255);
                } else if (pattern_time < DOUBLE_BLINK_ON_TIME + DOUBLE_BLINK_OFF_TIME) {
                    set_led_pwm(0);
                } else if (pattern_time < 2 * DOUBLE_BLINK_ON_TIME + DOUBLE_BLINK_OFF_TIME) {
                    set_led_pwm(255);
                } else if (pattern_time < 2 * (DOUBLE_BLINK_ON_TIME + DOUBLE_BLINK_OFF_TIME)) {
                    set_led_pwm(0);
                } else if (pattern_time < cycle_time) {
                    set_led_pwm(0); // Pause
                } else {
                    pattern_time = 0;
                }
            }
            break;
            
        case LED_PATTERN_TRIPLE_BLINK:
            {
                uint32_t cycle_time = 3 * (TRIPLE_BLINK_ON_TIME + TRIPLE_BLINK_OFF_TIME) + TRIPLE_BLINK_PAUSE_TIME;
                
                if (pattern_time < TRIPLE_BLINK_ON_TIME) {
                    set_led_pwm(255);
                } else if (pattern_time < TRIPLE_BLINK_ON_TIME + TRIPLE_BLINK_OFF_TIME) {
                    set_led_pwm(0);
                } else if (pattern_time < 2 * TRIPLE_BLINK_ON_TIME + TRIPLE_BLINK_OFF_TIME) {
                    set_led_pwm(255);
                } else if (pattern_time < 2 * (TRIPLE_BLINK_ON_TIME + TRIPLE_BLINK_OFF_TIME)) {
                    set_led_pwm(0);
                } else if (pattern_time < 3 * TRIPLE_BLINK_ON_TIME + 2 * TRIPLE_BLINK_OFF_TIME) {
                    set_led_pwm(255);
                } else if (pattern_time < 3 * (TRIPLE_BLINK_ON_TIME + TRIPLE_BLINK_OFF_TIME)) {
                    set_led_pwm(0);
                } else if (pattern_time < cycle_time) {
                    set_led_pwm(0); // Pause
                } else {
                    pattern_time = 0;
                }
            }
            break;
            
        case LED_PATTERN_SOS:
            {
                // Implementation of SOS Morse code pattern
                uint8_t code = sos_pattern[pattern_state];
                uint32_t duration;
                
                if (code == 0) {
                    // Element pause
                    duration = SOS_ELEMENT_PAUSE;
                    set_led_pwm(0);
                } else if (code == 1) {
                    // Dot
                    duration = SOS_DOT_TIME;
                    set_led_pwm(255);
                } else if (code == 2) {
                    // Dash
                    duration = SOS_DASH_TIME;
                    set_led_pwm(255);
                } else if (code == 3) {
                    // Word pause
                    duration = SOS_WORD_PAUSE;
                    set_led_pwm(0);
                } else {
                    duration = SOS_ELEMENT_PAUSE;
                    set_led_pwm(0);
                }
                
                if (pattern_time >= duration) {
                    pattern_time = 0;
                    pattern_state = (pattern_state + 1) % sizeof(sos_pattern);
                }
            }
            break;
    }
}

void led_set_brightness(uint8_t brightness) {
    current_brightness = brightness;
}

uint8_t led_get_brightness(void) {
    return current_brightness;
}

void led_set_on(uint8_t on) {
    if (on) {
        led_set_pattern(LED_PATTERN_ON);
    } else {
        led_set_pattern(LED_PATTERN_OFF);
    }
} 