/**
 * @file g722_codec.h
 * @brief G.722 audio codec implementation for Pico-ASHA
 */

#ifndef G722_CODEC_H
#define G722_CODEC_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief G.722 state structure
 */
typedef struct g722_encode_state_s g722_encode_state_t;
typedef struct g722_decode_state_s g722_decode_state_t;

/**
 * @brief Initialize a G.722 encoder
 * 
 * @param bit_rate The required bit rate (48000, 56000, or 64000)
 * @param options Encoding options (1 = packed format, 0 = shift format)
 * @return A pointer to the encoder state, or NULL for error
 */
g722_encode_state_t *g722_encoder_init(int bit_rate, int options);

/**
 * @brief Release a G.722 encoder
 * 
 * @param s The encoder state
 */
void g722_encoder_release(g722_encode_state_t *s);

/**
 * @brief Encode linear PCM data to G.722
 * 
 * @param s The encoder state
 * @param g722_data The G.722 data produced
 * @param pcm_data The PCM data to encode
 * @param len The number of PCM samples to encode
 * @return The number of G.722 bytes produced
 */
int g722_encode(g722_encode_state_t *s, uint8_t g722_data[], const int16_t pcm_data[], int len);

/**
 * @brief Initialize a G.722 decoder
 * 
 * @param bit_rate The required bit rate (48000, 56000, or 64000)
 * @param options Decoding options (1 = packed format, 0 = shift format)
 * @return A pointer to the decoder state, or NULL for error
 */
g722_decode_state_t *g722_decoder_init(int bit_rate, int options);

/**
 * @brief Release a G.722 decoder
 * 
 * @param s The decoder state
 */
void g722_decoder_release(g722_decode_state_t *s);

/**
 * @brief Decode G.722 data to linear PCM
 * 
 * @param s The decoder state
 * @param pcm_data The PCM data produced
 * @param g722_data The G.722 data to decode
 * @param len The number of G.722 bytes to decode
 * @return The number of PCM samples produced
 */
int g722_decode(g722_decode_state_t *s, int16_t pcm_data[], const uint8_t g722_data[], int len);

#ifdef __cplusplus
}
#endif

#endif /* G722_CODEC_H */ 