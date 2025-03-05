/**
 * @file hearing_aid.h
 * @brief Hearing aid interface for Pico-ASHA
 */

#ifndef HEARING_AID_H
#define HEARING_AID_H

#include <stdint.h>
#include <stdbool.h>
#include <btstack.h>

// Forward declaration to avoid circular includes
struct audio_stream_t;

/**
 * @brief Hearing aid state machine states
 */
typedef enum {
    HEARING_AID_STATE_DISCONNECTED,
    HEARING_AID_STATE_SCANNING,
    HEARING_AID_STATE_CONNECTING,
    HEARING_AID_STATE_SERVICE_DISCOVERY,
    HEARING_AID_STATE_CHARACTERISTICS_DISCOVERY,
    HEARING_AID_STATE_READY,
    HEARING_AID_STATE_STREAMING,
    HEARING_AID_STATE_DISCONNECTING,
    HEARING_AID_STATE_ERROR
} hearing_aid_state_t;

/**
 * @brief Audio state machine states
 */
typedef enum {
    AUDIO_STATE_IDLE,
    AUDIO_STATE_STARTING,
    AUDIO_STATE_STREAMING,
    AUDIO_STATE_STOPPING,
    AUDIO_STATE_ERROR
} audio_state_t;

/**
 * @brief Hearing aid ear side
 */
typedef enum {
    HEARING_AID_SIDE_UNKNOWN,
    HEARING_AID_SIDE_LEFT,
    HEARING_AID_SIDE_RIGHT,
    HEARING_AID_SIDE_BINAURAL
} hearing_aid_side_t;

/**
 * @brief Hearing aid type
 */
typedef enum {
    HEARING_AID_TYPE_UNKNOWN,
    HEARING_AID_TYPE_HEARING_AID,
    HEARING_AID_TYPE_COCHLEAR_IMPLANT
} hearing_aid_type_t;

/**
 * @brief Hearing aid capabilities
 */
typedef struct {
    bool supports_volume_control;
    bool supports_bass_treble;
    bool supports_mic_mute;
    bool supports_noise_reduction;
} hearing_aid_capabilities_t;

/**
 * @brief Hearing aid device information
 */
typedef struct {
    bd_addr_t address;
    uint8_t address_type;
    char name[32];
    char manufacturer[32];
    char model[32];
    char firmware_version[16];
    hearing_aid_side_t side;
    hearing_aid_type_t type;
    hearing_aid_capabilities_t capabilities;
    int8_t rssi;
    uint8_t bonded;
} hearing_aid_info_t;

/**
 * @brief Hearing aid device structure
 */
typedef struct {
    hearing_aid_info_t info;
    hearing_aid_state_t state;
    audio_state_t audio_state;
    hci_con_handle_t connection_handle;
    btstack_timer_source_t state_machine_timer;
    struct audio_stream_t* audio_stream;
    uint16_t asha_service_handle;
    uint16_t asha_read_psm_handle;
    uint16_t asha_audio_control_point_handle;
    uint16_t asha_volume_handle;
    uint16_t asha_le_psmvalue;
    uint16_t asha_cid;
    uint8_t asha_version;
    uint8_t volume;
    uint32_t start_time_ms;
    uint8_t retries;
    uint8_t error_code;
} hearing_aid_t;

/**
 * @brief Initialize the hearing aid module
 * 
 * @return 0 on success, non-zero on failure
 */
int hearing_aid_init(void);

/**
 * @brief Start scanning for hearing aids
 * 
 * @param timeout_ms Timeout in milliseconds, 0 for infinite
 * @return 0 on success, non-zero on failure
 */
int hearing_aid_start_scanning(uint32_t timeout_ms);

/**
 * @brief Stop scanning for hearing aids
 * 
 * @return 0 on success, non-zero on failure
 */
int hearing_aid_stop_scanning(void);

/**
 * @brief Connect to a hearing aid
 * 
 * @param address The Bluetooth address of the hearing aid
 * @param address_type The Bluetooth address type
 * @return 0 on success, non-zero on failure
 */
int hearing_aid_connect(bd_addr_t address, uint8_t address_type);

/**
 * @brief Connect to a previously bonded hearing aid
 * 
 * @return 0 on success, non-zero on failure
 */
int hearing_aid_connect_bonded(void);

/**
 * @brief Disconnect from the current hearing aid
 * 
 * @return 0 on success, non-zero on failure
 */
int hearing_aid_disconnect(void);

/**
 * @brief Start audio streaming to the hearing aid
 * 
 * @return 0 on success, non-zero on failure
 */
int hearing_aid_start_audio(void);

/**
 * @brief Stop audio streaming to the hearing aid
 * 
 * @return 0 on success, non-zero on failure
 */
int hearing_aid_stop_audio(void);

/**
 * @brief Set the volume (0-100)
 * 
 * @param volume The volume level (0-100)
 * @return 0 on success, non-zero on failure
 */
int hearing_aid_set_volume(uint8_t volume);

/**
 * @brief Get the current volume (0-100)
 * 
 * @return The current volume level (0-100)
 */
uint8_t hearing_aid_get_volume(void);

/**
 * @brief Process the hearing aid state machine
 * 
 * @param force_run Force the state machine to run
 */
void hearing_aid_process(bool force_run);

/**
 * @brief Get the current state
 * 
 * @return The current state
 */
hearing_aid_state_t hearing_aid_get_state(void);

/**
 * @brief Get the current audio state
 * 
 * @return The current audio state
 */
audio_state_t hearing_aid_get_audio_state(void);

/**
 * @brief Get string representation of a state
 * 
 * @param state The state to convert to string
 * @return String representation of the state
 */
const char* hearing_aid_state_to_string(hearing_aid_state_t state);

/**
 * @brief Get string representation of an audio state
 * 
 * @param state The audio state to convert to string
 * @return String representation of the audio state
 */
const char* hearing_aid_audio_state_to_string(audio_state_t state);

/**
 * @brief Check if the connection is established
 * 
 * @return true if connected, false otherwise
 */
bool hearing_aid_is_connected(void);

/**
 * @brief Event callback for BLE events
 * 
 * @param packet_type The packet type
 * @param channel The channel
 * @param packet The packet
 * @param size The size of the packet
 */
void hearing_aid_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

#endif /* HEARING_AID_H */ 