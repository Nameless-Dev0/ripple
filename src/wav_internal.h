#ifndef WAV_INTERNAL_H
#define WAV_INTERNAL_H

#include <stdio.h>
#include <stdint.h>

typedef struct wav_s{
    uint32_t duration; /* In seconds */
    
    /* canonical wav format fields */
    char chunk_id[5];
    uint32_t chunk_size;
    char format[5];
    char sub_chunk1_id[5];
    uint32_t sub_chunk1_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bit_depth; /* bits per sample */
    char sub_chunk2_id[5];
    uint32_t sub_chunk2_size;
    uint8_t* data; /* points to start of audio samples */
} wav_t;

uint16_t read_le16(const uint8_t* p);  /* Converts host 16-bit integer to little endian    */
uint32_t read_le32(const uint8_t* p);  /* Converts host 32-bit integer to little endian */
void write_le32(const uint8_t *p, FILE* file);  /* Writes to 32-bit integer to file in little endian */ 
void write_le16(const uint8_t *p, FILE* file);  /* Writes to 16-bit integer to file in little endian */ 

#endif /* WAV_INTERNAL_H */