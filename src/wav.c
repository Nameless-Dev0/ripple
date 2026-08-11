#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <ctype.h>
#include "wav.h"

static inline void bytes_to_ascii(const char *src, char *dest); /* Helper to convert raw bytes (src) into an ASCII string (dest) */
static inline uint32_t to_little_endian32(uint32_t val);        /* Converts host 32-bit integer to Little-Endian representation */
static inline uint32_t to_big_endian32(uint32_t val);           /*  */

typedef struct wav_s{
    FILE* wav_file;

    /* Canonical WAV format fields */
    uint32_t chunk_id;
    uint32_t chunk_size;
    uint32_t format;
    uint32_t sub_chunk1_id;
    uint32_t sub_chunk1_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bit_depth; /* bits per sample */
    uint32_t sub_chunk2_id;
    uint32_t sub_chunk2_size;
    uint32_t* data; /* Points to start of audio samples */
} wav_t;

uint32_t to_big_endian32(uint32_t val) {
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return __builtin_bswap32(val);
#else
    return val;
#endif
}

uint32_t to_little_endian32(uint32_t val) {
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return __builtin_bswap32(val);
#else
    return val;
#endif
}

void bytes_to_ascii(const char *src, char *dest) {
    if (!src || !dest) return;

    for (size_t i = 0; i < 4; i++) {
        unsigned char c = (unsigned char)src[i];
        dest[i] = isprint(c) ? (char)c : '.'; /* '.' is a placeholder for non-printable characters */
    }
    dest[4] = '\0';
}


wav_t* load_wav(FILE* wav_file){
    /* TODO */
}

wav_status_t read_wav(wav_t* wav){
    /* TODO */
}

wav_status_t release_wav(wav_t* wav){
    /* TODO */
}

wav_status_t export_wav(const wav_t* wav, FILE* out_file){
    /* TODO */
}
