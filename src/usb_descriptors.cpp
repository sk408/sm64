/**
 * @file usb_descriptors.cpp
 * @brief USB descriptors implementation for Pico-ASHA
 */

#include <string.h>
#include <tusb.h>
#include <pico/stdlib.h>
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

// USB configuration descriptor
static const uint8_t config_descriptor[] = {
    // Configuration descriptor
    TUD_CONFIG_DESCRIPTOR(1, 3, 0, 200, 0x00, 100),
    
    // Audio control interface
    TUD_AUDIO_DESC_IAD(AUDIO_CTRL_INTERFACE, 2, TUSB_CLASS_AUDIO, AUDIO_SUBCLASS_CONTROL, AUDIO_PROTOCOL_V2, 0),
    TUD_AUDIO_DESC_STD_AC(AUDIO_CTRL_INTERFACE, 0, 1, AUDIO_FUNCTION_CATEGORY_HEADSET, 0, 0),
    TUD_AUDIO_DESC_CS_AC(0x01, 0x01, 0x00, 1),
    
    // Audio streaming interface
    TUD_AUDIO_DESC_STD_AS_INT(AUDIO_STREAMING_INTERFACE, 0, 1, TUSB_CLASS_AUDIO, AUDIO_SUBCLASS_STREAMING, AUDIO_PROTOCOL_V2),
    TUD_AUDIO_DESC_STD_AS_INT_ALT(AUDIO_STREAMING_INTERFACE, 1, 1, TUSB_CLASS_AUDIO, AUDIO_SUBCLASS_STREAMING, AUDIO_PROTOCOL_V2),
    TUD_AUDIO_DESC_CS_AS_INT(0x01, 0x01, AUDIO_FORMAT_TYPE_I, AUDIO_DATA_FORMAT_PCM, AUDIO_CHANNELS, AUDIO_BIT_RESOLUTION, AUDIO_SAMPLE_RATE),
    TUD_AUDIO_DESC_STD_AS_ISO_EP(AUDIO_STREAMING_EP, 3, AUDIO_BUFFER_SIZE, 1, (CFG_TUSB_RHPORT0_MODE & OPT_MODE_HIGH_SPEED) ? 8 : 1),
    
    // CDC Interface for logging
    TUD_CDC_DESCRIPTOR(CDC_INTERFACE, 4, CDC_NOTIF_EP, 8, CDC_DATA_EP, 64)
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
    if (!usb_connected || !tud_audio_mounted()) {
        return 0;
    }
    
    uint32_t sent = tud_audio_write(data, len);
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

// Invoked when audio class specific get request received for an entity
bool tud_audio_get_req_cb(uint8_t rhport, tusb_control_request_t const *request) {
    // Handle audio control requests
    return false; // Not implemented
}

// Invoked when audio class specific set request received for an entity
bool tud_audio_set_req_cb(uint8_t rhport, tusb_control_request_t const *request) {
    // Handle volume control
    uint8_t const itf = request->wIndex;
    uint8_t const ep = request->wIndex >> 8;
    
    if (request->bRequest == AUDIO_CS_REQ_CUR) {
        if (request->wValue == (AUDIO_CS_MUTE_CONTROL << 8)) {
            // Mute control
            uint8_t mute;
            tud_audio_buffer_and_schedule_control_xfer(rhport, request, &mute, 1);
            return true;
        } else if (request->wValue == (AUDIO_CS_VOLUME_CONTROL << 8)) {
            // Volume control
            uint16_t volume;
            tud_audio_buffer_and_schedule_control_xfer(rhport, request, &volume, 2);
            
            // Convert to 0-100 scale
            audio_volume = (uint8_t)(volume * 100 / UINT16_MAX);
            LOG_DEBUG("USB volume set to %d", audio_volume);
            return true;
        }
    }
    
    return false;
}

// Invoked when audio is received from the host
void tud_audio_rx_cb(uint8_t rhport, uint8_t *buf, uint16_t count) {
    // Not implemented for our audio device (we only send audio, not receive)
}

// Invoked when CDC data is received from the host
void tud_cdc_rx_cb(uint8_t itf) {
    // Read and process any received CDC data
    uint8_t buf[64];
    uint32_t count = tud_cdc_read(buf, sizeof(buf));
    
    if (count > 0) {
        // Process received commands here
        // For now, just echo back
        tud_cdc_write(buf, count);
        tud_cdc_write_flush();
    }
}

// Invoked when cdc line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
    // Connected = DTR
    LOG_INFO("CDC line state: DTR=%d, RTS=%d", dtr, rts);
}

// Invoked when CDC line coding changed
void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* line_coding) {
    LOG_INFO("CDC line coding: baud=%lu, format=%u, parity=%u, data bits=%u", 
        line_coding->bit_rate, line_coding->stop_bits, line_coding->parity, line_coding->data_bits);
}

// Invoked when device is mounted (configured)
void tud_mount_cb(void) {
    LOG_INFO("USB device mounted");
    usb_connected = true;
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
    LOG_INFO("USB device unmounted");
    usb_connected = false;
}

// Invoked when usb bus is suspended
void tud_suspend_cb(bool remote_wakeup_en) {
    LOG_INFO("USB suspended");
    usb_connected = false;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
    LOG_INFO("USB resumed");
    usb_connected = tud_mounted();
}

// Device descriptor callback
uint8_t const* tud_descriptor_device_cb(void) {
    return (uint8_t const*)&device_descriptor;
}

// Configuration descriptor callback
uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    return config_descriptor;
}

// String descriptor callback
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
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