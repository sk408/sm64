/**
 * @file usb_descriptors.h
 * @brief USB descriptors for Pico-ASHA
 */

#ifndef USB_DESCRIPTORS_H
#define USB_DESCRIPTORS_H

#include <stdint.h>
#include <tusb.h>

// Audio control interfaces
#define AUDIO_CTRL_INTERFACE      0
#define AUDIO_STREAMING_INTERFACE 1
#define CDC_INTERFACE             2  // CDC COM port interface

// Endpoints
#define AUDIO_STREAMING_EP        0x01
#define CDC_NOTIF_EP              0x81
#define CDC_DATA_EP               0x02

// Audio format settings
#define AUDIO_SAMPLE_RATE         16000  // 16 kHz
#define AUDIO_CHANNELS            2      // Stereo
#define AUDIO_BIT_RESOLUTION      16     // 16-bit
#define AUDIO_BYTE_RESOLUTION     (AUDIO_BIT_RESOLUTION / 8)
#define AUDIO_FRAME_SIZE          (AUDIO_CHANNELS * AUDIO_BYTE_RESOLUTION)

// Audio buffer size (in samples)
#define AUDIO_BUFFER_SAMPLES      (AUDIO_SAMPLE_RATE / 100) // 10 ms buffer
#define AUDIO_BUFFER_SIZE         (AUDIO_BUFFER_SAMPLES * AUDIO_FRAME_SIZE)

// USB device descriptor strings
extern const char* usb_strings[];

// USB device callbacks
bool tud_audio_tx_done_pre_load_cb(uint8_t rhport, uint8_t itf, uint8_t ep_in, uint8_t cur_alt_setting);
bool tud_audio_tx_done_post_load_cb(uint8_t rhport, uint8_t itf, uint8_t ep_in, uint8_t cur_alt_setting);
bool tud_audio_set_itf_close_EP_cb(uint8_t rhport, tusb_control_request_t const *request);

// Audio class callbacks
bool tud_audio_get_req_cb(uint8_t rhport, tusb_control_request_t const *request);
bool tud_audio_set_req_cb(uint8_t rhport, tusb_control_request_t const *request);

// Audio data callbacks
void tud_audio_rx_cb(uint8_t rhport, uint8_t *buf, uint16_t count);

// CDC class callbacks
void tud_cdc_rx_cb(uint8_t itf);
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts);
void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* line_coding);

/**
 * @brief Initialize USB descriptors
 */
void usb_descriptors_init(void);

/**
 * @brief Send audio data to USB host
 * 
 * @param data Data to send
 * @param len Length of data in bytes
 * @return Number of bytes sent, or 0 if none sent
 */
uint32_t usb_audio_send(const uint8_t* data, uint32_t len);

/**
 * @brief Get USB audio volume
 * 
 * @return Volume (0-100)
 */
uint8_t usb_audio_get_volume(void);

/**
 * @brief Check if USB is connected and ready
 * 
 * @return true if connected and ready, false otherwise
 */
bool usb_is_connected(void);

/**
 * @brief Send CDC data to USB host
 * 
 * @param data Data to send
 * @param len Length of data in bytes
 * @return Number of bytes sent, or 0 if none sent
 */
uint32_t usb_cdc_send(const uint8_t* data, uint32_t len);

/**
 * @brief Process USB tasks (call in a loop)
 */
void usb_process(void);

#endif /* USB_DESCRIPTORS_H */ 