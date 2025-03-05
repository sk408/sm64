/**
 * @file main.cpp
 * @brief Main application for Pico-ASHA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <hardware/flash.h>
#include <hardware/sync.h>
#include "asha_logging.h"
#include "asha_led.h"
#include "hearing_aid.h"
#include "asha_bt.h"
#include "asha_audio.h"
#include "usb_descriptors.h"

// Global variables shared between cores
static volatile bool g_usb_connected = false;
static volatile bool g_bt_initialized = false;
static volatile bool g_app_running = true;

// Mutex for shared resources
static mutex_t g_mutex;

// Audio stream
static audio_stream_t *g_audio_stream = NULL;

// System time for LED processing
static uint32_t g_last_led_time = 0;

/**
 * @brief Task to handle USB and audio processing (runs on core 0)
 */
static void core0_task(void) {
    uint32_t current_time;
    uint32_t elapsed_time;
    
    LOG_INFO("Core 0 task started");
    
    // Initialize USB
    usb_descriptors_init();
    LOG_INFO("USB initialized");
    
    // Initialize LED
    led_init();
    led_set_pattern(LED_PATTERN_BLINK_SLOW);
    LOG_INFO("LED initialized");
    
    // Initialize audio system
    audio_init();
    g_audio_stream = audio_create_stream(AUDIO_SAMPLE_RATE, AUDIO_FORMAT_PCM_16BIT, AUDIO_CHANNEL_STEREO);
    if (g_audio_stream == NULL) {
        LOG_ERROR("Failed to create audio stream");
        led_set_pattern(LED_PATTERN_SOS);
        while (1) {
            led_process(10);
            sleep_ms(10);
        }
    }
    LOG_INFO("Audio initialized");
    
    // Main loop
    g_last_led_time = to_ms_since_boot(get_absolute_time());
    
    while (g_app_running) {
        // Process USB
        usb_process();
        
        // Check USB connection status
        bool usb_status = usb_is_connected();
        if (usb_status != g_usb_connected) {
            g_usb_connected = usb_status;
            LOG_INFO("USB connection status: %s", g_usb_connected ? "connected" : "disconnected");
            
            if (g_usb_connected) {
                led_set_pattern(LED_PATTERN_BLINK_FAST);
            } else {
                led_set_pattern(LED_PATTERN_BLINK_SLOW);
            }
        }
        
        // Process audio if connected
        if (g_usb_connected && g_bt_initialized) {
            // Lock mutex to access shared resources
            mutex_enter_blocking(&g_mutex);
            
            // Process audio data
            audio_process(g_audio_stream);
            
            // Release mutex
            mutex_exit(&g_mutex);
        }
        
        // Update LED
        current_time = to_ms_since_boot(get_absolute_time());
        elapsed_time = current_time - g_last_led_time;
        g_last_led_time = current_time;
        led_process(elapsed_time);
        
        // Yield to other tasks
        sleep_ms(1);
    }
}

/**
 * @brief Task to handle Bluetooth operations (runs on core 1)
 */
static void core1_task(void) {
    LOG_INFO("Core 1 task started");
    
    // Initialize Bluetooth
    if (asha_bt_init() != 0) {
        LOG_ERROR("Failed to initialize Bluetooth");
        return;
    }
    LOG_INFO("Bluetooth initialized");
    
    // Set up hearing aid module
    if (hearing_aid_init() != 0) {
        LOG_ERROR("Failed to initialize hearing aid module");
        return;
    }
    LOG_INFO("Hearing aid module initialized");
    
    // Signal that Bluetooth is initialized
    g_bt_initialized = true;
    
    // Main loop
    while (g_app_running) {
        // Process Bluetooth events
        asha_bt_process();
        
        // Process hearing aid state machine
        hearing_aid_process(false);
        
        // Yield to other tasks
        sleep_ms(1);
    }
}

/**
 * @brief Main application entry point
 */
int main(void) {
    // Initialize stdio
    stdio_init_all();
    
    // Wait for USB to initialize
    sleep_ms(2000);
    
    // Initialize logging
    logging_init(LOG_LEVEL_INFO);
    LOG_INFO("Pico-ASHA starting...");
    
    // Initialize mutex
    mutex_init(&g_mutex);
    
    // Start core 1 (Bluetooth processing)
    LOG_INFO("Starting core 1 (Bluetooth)");
    multicore_launch_core1(core1_task);
    
    // Run core 0 task (USB and audio processing)
    core0_task();
    
    // Should never reach here
    return 0;
} 