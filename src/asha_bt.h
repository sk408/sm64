/**
 * @file asha_bt.h
 * @brief Bluetooth implementation for ASHA
 */

#ifndef ASHA_BT_H
#define ASHA_BT_H

#include <stdint.h>
#include <stdbool.h>
#include <btstack.h>

// ASHA UUIDs
#define ASHA_SERVICE_UUID                    0xFDF0
#define ASHA_READ_PSM_UUID                   0xFDF1
#define ASHA_AUDIO_CONTROL_POINT_UUID        0xFDF2
#define ASHA_AUDIO_STATUS_UUID               0xFDF3
#define ASHA_VOLUME_UUID                     0xFDF4
#define ASHA_LE_PSM_UUID                     0xFDF5

// ASHA constants
#define ASHA_PROTOCOL_VERSION                0x01
#define ASHA_CODEC_ID_G722_16KHZ             0x01
#define ASHA_MAX_VOLUME                      100

// ASHA Audio Control Point commands
#define ASHA_COMMAND_START                   0x01
#define ASHA_COMMAND_STOP                    0x02
#define ASHA_COMMAND_STATUS                  0x03

// ASHA Audio Status values
#define ASHA_STATUS_INACTIVE                 0x00
#define ASHA_STATUS_ACTIVE                   0x01
#define ASHA_STATUS_STREAMING_ACTIVE         0x02
#define ASHA_STATUS_STREAMING_SUSPENDED      0x03

/**
 * @brief Initialize Bluetooth for ASHA
 * 
 * @return 0 on success, non-zero on failure
 */
int asha_bt_init(void);

/**
 * @brief Start the Bluetooth stack
 * 
 * @return 0 on success, non-zero on failure
 */
int asha_bt_start(void);

/**
 * @brief Process Bluetooth events (call in a loop)
 */
void asha_bt_process(void);

/**
 * @brief Start scanning for ASHA-compatible devices
 * 
 * @param timeout_ms Timeout in milliseconds, 0 for infinite
 * @return 0 on success, non-zero on failure
 */
int asha_bt_start_scan(uint32_t timeout_ms);

/**
 * @brief Stop scanning for ASHA-compatible devices
 * 
 * @return 0 on success, non-zero on failure
 */
int asha_bt_stop_scan(void);

/**
 * @brief Connect to an ASHA-compatible device
 * 
 * @param address The Bluetooth address of the device
 * @param address_type The Bluetooth address type
 * @return 0 on success, non-zero on failure
 */
int asha_bt_connect(bd_addr_t address, uint8_t address_type);

/**
 * @brief Connect to a previously bonded device
 * 
 * @return 0 on success, non-zero on failure
 */
int asha_bt_connect_bonded(void);

/**
 * @brief Disconnect from the current device
 * 
 * @param handle The connection handle
 * @return 0 on success, non-zero on failure
 */
int asha_bt_disconnect(hci_con_handle_t handle);

/**
 * @brief Start discovering services on a connected device
 * 
 * @param handle The connection handle
 * @return 0 on success, non-zero on failure
 */
int asha_bt_discover_services(hci_con_handle_t handle);

/**
 * @brief Write to the Audio Control Point
 * 
 * @param handle The connection handle
 * @param acp_handle The Audio Control Point handle
 * @param command The command to write
 * @param value The value to write
 * @return 0 on success, non-zero on failure
 */
int asha_bt_write_audio_control_point(hci_con_handle_t handle, uint16_t acp_handle, uint8_t command, uint8_t value);

/**
 * @brief Set volume
 * 
 * @param handle The connection handle
 * @param volume_handle The Volume handle
 * @param volume The volume level (0-100)
 * @return 0 on success, non-zero on failure
 */
int asha_bt_set_volume(hci_con_handle_t handle, uint16_t volume_handle, uint8_t volume);

/**
 * @brief Create an L2CAP CoC (Connection Oriented Channel) for audio streaming
 * 
 * @param handle The connection handle
 * @param psm The PSM value
 * @return 0 on success, non-zero on failure
 */
int asha_bt_create_l2cap_channel(hci_con_handle_t handle, uint16_t psm);

/**
 * @brief Send audio data over L2CAP CoC
 * 
 * @param cid The L2CAP channel ID
 * @param data The audio data
 * @param length The length of the audio data
 * @return 0 on success, non-zero on failure
 */
int asha_bt_send_audio_data(uint16_t cid, uint8_t *data, uint16_t length);

/**
 * @brief Close an L2CAP CoC
 * 
 * @param cid The L2CAP channel ID
 * @return 0 on success, non-zero on failure
 */
int asha_bt_close_l2cap_channel(uint16_t cid);

/**
 * @brief Register for ASHA BLE events
 * 
 * @param callback The callback function
 * @return 0 on success, non-zero on failure
 */
int asha_bt_register_callback(btstack_packet_handler_t callback);

/**
 * @brief Check if the Bluetooth stack is initialized
 * 
 * @return true if initialized, false otherwise
 */
bool asha_bt_is_initialized(void);

/**
 * @brief Check if the device is currently scanning
 * 
 * @return true if scanning, false otherwise
 */
bool asha_bt_is_scanning(void);

/**
 * @brief Check if there is an active connection
 * 
 * @return true if connected, false otherwise
 */
bool asha_bt_is_connected(void);

/**
 * @brief Get the device name
 * 
 * @return The device name
 */
const char* asha_bt_get_device_name(void);

/**
 * @brief Set the device name
 * 
 * @param name The device name
 * @return 0 on success, non-zero on failure
 */
int asha_bt_set_device_name(const char* name);

#endif /* ASHA_BT_H */ 