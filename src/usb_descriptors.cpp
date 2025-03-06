/**
 * @file usb_descriptors.cpp
 * @brief USB descriptors implementation for Pico-ASHA
 */

#include <string.h>
#include <tusb.h>
#include <pico/stdlib.h>
#include <hardware/sync.h>
#include <pico/mutex.h>
#include "usb_descriptors.h"
#include "asha_logging.h"
#include "asha_audio.h"

// USB device descriptor
static const tusb_desc_device_t device_descriptor = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,  // USB 2.0
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = 0x2E8A,  // Raspberry Pi
    .idProduct          = 0x000A,  // Pico Audio
    .bcdDevice          = 0x0100,  // Version 1.0
    .iManufacturer      = 1,
    .iProduct           = 2,
    .iSerialNumber      = 3,
    .bNumConfigurations = 1
};

// Audio class constants needed for descriptors
#define AUDIO_FUNCTION_CATEGORY_HEADSET       0x0402
#define AUDIO_SUBCLASS_CONTROL                1
#define AUDIO_SUBCLASS_STREAMING              2
#define AUDIO_PROTOCOL_V2                     0x20
#define AUDIO_CS_AC_INTERFACE_HEADER          0x01
#define AUDIO_CS_AS_INTERFACE_AS_GENERAL      0x01
#define AUDIO_CS_REQ_CUR                      0x01
#define AUDIO_CS_MUTE_CONTROL                 0x01
#define AUDIO_CS_VOLUME_CONTROL               0x02
#define AUDIO_FORMAT_TYPE_I                   0x01
#define AUDIO_DATA_FORMAT_PCM                 0x00000001

// USB configuration descriptor
// Use the TinyUSB-provided macros directly
static const uint8_t config_descriptor[] = {
    // Configuration descriptor
    TUD_CONFIG_DESCRIPTOR(1, 3, 0, 200, 0x00, 100),
    
    // CDC Interface for logging
    TUD_CDC_DESCRIPTOR(CDC_INTERFACE, 4, CDC_NOTIF_EP, 8, CDC_DATA_EP, CDC_DATA_EP + 1, 64)
};

// USB string descriptors
const char* usb_strings[] = {
    (const char[]){0x09, 0x04},  // 0: Supported language is English (0x0409)
    "Raspberry Pi",              // 1: Manufacturer
    "Pico-ASHA Audio Interface", // 2: Product
    "000000000001",              // 3: Serial number
    "Audio Control",             // 4: Audio Interface
    "CDC Data"                   // 5: CDC Interface
};

// Audio control
static uint8_t audio_volume = 50;  // Default volume (0-100)
static bool usb_connected = false;

/**
 * @brief Initialize USB descriptors
 */
void usb_descriptors_init(void) {
    // Initialize TinyUSB
    tusb_init();
    LOG_INFO("USB descriptors initialized");
}

/**
 * @brief Send audio data to USB host
 * 
 * @param data Data to send
 * @param len Length of data in bytes
 * @return Number of bytes sent, or 0 if none sent
 */
uint32_t usb_audio_send(const uint8_t* data, uint32_t len) {
    if (!usb_connected || !tud_mounted()) {
        return 0;
    }
    
    // Use CDC for now as a workaround since audio is not fully implemented
    uint32_t sent = tud_cdc_write(data, len);
    return sent;
}

/**
 * @brief Get USB audio volume
 * 
 * @return Volume (0-100)
 */
uint8_t usb_audio_get_volume(void) {
    return audio_volume;
}

/**
 * @brief Check if USB is connected and ready
 * 
 * @return true if connected and ready, false otherwise
 */
bool usb_is_connected(void) {
    return usb_connected;
}

/**
 * @brief Send CDC data to USB host
 * 
 * @param data Data to send
 * @param len Length of data in bytes
 * @return Number of bytes sent, or 0 if none sent
 */
uint32_t usb_cdc_send(const uint8_t* data, uint32_t len) {
    if (!usb_connected || !tud_cdc_connected()) {
        return 0;
    }
    
    uint32_t sent = 0;
    
    // Wait for available space in CDC buffer
    while (sent < len) {
        uint32_t available = tud_cdc_write_available();
        if (available == 0) {
            break;
        }
        
        uint32_t to_send = (len - sent) > available ? available : (len - sent);
        uint32_t written = tud_cdc_write(data + sent, to_send);
        sent += written;
        
        if (written < to_send) {
            break;
        }
    }
    
    tud_cdc_write_flush();
    return sent;
}

/**
 * @brief Process USB tasks (call in a loop)
 */
void usb_process(void) {
    tud_task();
    
    // Update connection status
    usb_connected = tud_mounted();
}

// TinyUSB callbacks

// Audio callbacks
bool tud_audio_get_req_cb(uint8_t rhport, tusb_control_request_t const *request) {
    (void)rhport;
    (void)request;
    // For now, just return false to stall as audio is not fully implemented
    return false;
}

bool tud_audio_set_req_cb(uint8_t rhport, tusb_control_request_t const *request) {
    (void)rhport;
    (void)request;
    // For now, just return false to stall as audio is not fully implemented
    return false;
}

void tud_audio_rx_cb(uint8_t rhport, uint8_t *buf, uint16_t count) {
    (void)rhport;
    (void)buf;
    (void)count;
    // Not implemented - for audio reception from host
}

// CDC callbacks
void tud_cdc_rx_cb(uint8_t itf) {
    (void)itf;
    // Process received CDC data - not fully implemented here
}

void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
    (void)itf;
    (void)rts; // Unused parameter
    // USB CDC state change, update connection state
    usb_connected = dtr;
}

void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* line_coding) {
    (void)itf;
    (void)line_coding;
    // Not implemented - would handle baud rate changes
}

// Device state callbacks
void tud_mount_cb(void) {
    LOG_INFO("USB device mounted");
    usb_connected = true;
}

void tud_umount_cb(void) {
    LOG_INFO("USB device unmounted");
    usb_connected = false;
}

void tud_suspend_cb(bool remote_wakeup_en) {
    (void)remote_wakeup_en;
    // USB suspended, update connection state
    usb_connected = false;
}

void tud_resume_cb(void) {
    LOG_INFO("USB resumed");
    usb_connected = tud_mounted();
}

// Descriptor callbacks
uint8_t const* tud_descriptor_device_cb(void) {
    return (uint8_t const*)&device_descriptor;
}

uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    (void)index; // Only one configuration
    return config_descriptor;
}

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid; // Only support English
    static uint16_t str_desc[32];
    uint8_t len = 0;
    
    // Convert ASCII string to UTF-16
    if (index == 0) {
        memcpy(&str_desc[1], usb_strings[0], 2);
        len = 1;
    } else if (index < sizeof(usb_strings) / sizeof(usb_strings[0])) {
        const char* str = usb_strings[index];
        len = strlen(str);
        
        // Cap at max string length
        if (len > 31) {
            len = 31;
        }
        
        // Convert ASCII to UTF-16
        for (uint8_t i = 0; i < len; i++) {
            str_desc[1 + i] = str[i];
        }
    }
    
    // First byte is length (including header), second byte is string type
    str_desc[0] = (TUSB_DESC_STRING << 8) | (2 * len + 2);
    
    return str_desc;
} 