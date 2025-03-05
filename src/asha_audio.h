/**
 * @file asha_audio.h
 * @brief Audio processing for Pico-ASHA
 */

#ifndef ASHA_AUDIO_H
#define ASHA_AUDIO_H

#include <stdint.h>
#include <stdbool.h>
#include <lib/g722_codec/g722_codec.h>

/**
 * @brief Audio sample format
 */
typedef enum {
    AUDIO_FORMAT_PCM_16BIT = 0,
    AUDIO_FORMAT_PCM_24BIT,
    AUDIO_FORMAT_PCM_32BIT,
    AUDIO_FORMAT_FLOAT
} audio_format_t;

/**
 * @brief Audio channel configuration
 */
typedef enum {
    AUDIO_CHANNEL_MONO = 0,
    AUDIO_CHANNEL_STEREO,
    AUDIO_CHANNEL_LEFT_ONLY,
    AUDIO_CHANNEL_RIGHT_ONLY
} audio_channel_t;

/**
 * @brief Audio ring buffer for streaming
 */
typedef struct {
    uint8_t *buffer;
    size_t buffer_size;
    size_t read_index;
    size_t write_index;
    size_t data_size;
    bool overflow;
    bool underflow;
} audio_ring_buffer_t;

/**
 * @brief Audio stream structure
 */
typedef struct audio_stream_t {
    // Stream configuration
    uint32_t sample_rate;
    audio_format_t format;
    audio_channel_t channels;
    
    // G.722 codec state
    g722_encode_state_t *g722_encoder;
    
    // Ring buffers for left and right audio channels
    audio_ring_buffer_t left_buffer;
    audio_ring_buffer_t right_buffer;
    audio_ring_buffer_t encoded_buffer;
    
    // Stream control
    bool stream_active;
    uint8_t volume;
    
    // Stream statistics
    uint32_t frames_processed;
    uint32_t frames_encoded;
    uint32_t frames_sent;
    uint32_t underruns;
    uint32_t overruns;
} audio_stream_t;

/**
 * @brief Initialize the audio system
 * 
 * @return 0 on success, non-zero on failure
 */
int audio_init(void);

/**
 * @brief Create a new audio stream
 * 
 * @param sample_rate The sample rate in Hz
 * @param format The audio format
 * @param channels The channel configuration
 * @return A pointer to the new audio stream, or NULL on failure
 */
audio_stream_t* audio_create_stream(uint32_t sample_rate, audio_format_t format, audio_channel_t channels);

/**
 * @brief Destroy an audio stream
 * 
 * @param stream The audio stream to destroy
 */
void audio_destroy_stream(audio_stream_t *stream);

/**
 * @brief Start audio streaming
 * 
 * @param stream The audio stream to start
 * @return 0 on success, non-zero on failure
 */
int audio_start_stream(audio_stream_t *stream);

/**
 * @brief Stop audio streaming
 * 
 * @param stream The audio stream to stop
 * @return 0 on success, non-zero on failure
 */
int audio_stop_stream(audio_stream_t *stream);

/**
 * @brief Write audio data to the stream
 * 
 * @param stream The audio stream to write to
 * @param data The audio data to write
 * @param size The size of the audio data in bytes
 * @return The number of bytes written, or -1 on failure
 */
int audio_write(audio_stream_t *stream, const void *data, size_t size);

/**
 * @brief Read encoded audio data from the stream
 * 
 * @param stream The audio stream to read from
 * @param data The buffer to read into
 * @param size The size of the buffer in bytes
 * @return The number of bytes read, or -1 on failure
 */
int audio_read_encoded(audio_stream_t *stream, void *data, size_t size);

/**
 * @brief Process audio data (called in a loop)
 * 
 * @param stream The audio stream to process
 * @return 0 on success, non-zero on failure
 */
int audio_process(audio_stream_t *stream);

/**
 * @brief Set the volume of the audio stream
 * 
 * @param stream The audio stream to set the volume for
 * @param volume The volume level (0-100)
 * @return 0 on success, non-zero on failure
 */
int audio_set_volume(audio_stream_t *stream, uint8_t volume);

/**
 * @brief Get the volume of the audio stream
 * 
 * @param stream The audio stream to get the volume for
 * @return The volume level (0-100)
 */
uint8_t audio_get_volume(audio_stream_t *stream);

/**
 * @brief Check if the audio stream is active
 * 
 * @param stream The audio stream to check
 * @return true if active, false otherwise
 */
bool audio_is_active(audio_stream_t *stream);

/**
 * @brief Get the available encoded data size
 * 
 * @param stream The audio stream to check
 * @return The number of bytes available
 */
size_t audio_get_encoded_available(audio_stream_t *stream);

/**
 * @brief Get available space for writing
 * 
 * @param stream The audio stream to check
 * @return The number of bytes available
 */
size_t audio_get_write_available(audio_stream_t *stream);

/**
 * @brief Reset the audio stream
 * 
 * @param stream The audio stream to reset
 * @return 0 on success, non-zero on failure
 */
int audio_reset(audio_stream_t *stream);

#endif /* ASHA_AUDIO_H */ 