#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include "wav.h"

static inline void bytes_to_ascii(uint32_t src, char dest[5]);  /* Converts raw bytes (src) into an ASCII string (dest) */
static inline uint32_t read_be32(const uint8_t *p);             /* Converts host 32-bit integer to big endian representation */
static inline uint32_t read_le32(const uint8_t *p);             /* Converts host 32-bit integer to little endian representation */
static inline uint16_t read_le16(const uint8_t *p);             /* Converts host 16-bit integer to little endian representation */

typedef struct wav_s{
    /* canonical wav format fields */
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
    uint8_t* data; /* points to start of audio samples */
} wav_t;

static inline uint32_t read_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8)  |  (uint32_t)p[3];
}
static inline uint32_t read_le32(const uint8_t *p) {
    return  (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static inline uint16_t read_le16(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

void bytes_to_ascii(uint32_t src, char dest[5]) {
    if (!dest) {
        return;
    }
    for (size_t i = 0; i < 4; i++) {
        uint32_t shift = (3 - i) * 8;
        unsigned char c = (src >> shift) & 0xFF;
        dest[i] = isprint(c) ? (char)c : '.';
    }
    dest[4] = '\0';
}

wav_t* load_wav(const char* wav_file){
    FILE* wav_handle = fopen(wav_file, "rb");
    if(!wav_handle){
        perror("Cannot open file!");
        return NULL;
    }

    wav_t* wav = malloc(sizeof(struct wav_s));
    if(!wav){
        perror("Cannot allocate wav object!");
        fclose(wav_handle);
        return NULL;
    }

    uint8_t metadata[44];
    if(sizeof(metadata) != fread(metadata,1, sizeof(metadata), wav_handle)){
        perror("Unable to extract WAV metadata!");
        free(wav);
        fclose(wav_handle);
        return NULL;
    }

    /* RIFF HEADER */
    wav->chunk_id        = read_be32(&metadata[0]);
    wav->chunk_size      = read_le32(&metadata[4]);
    wav->format          = read_be32(&metadata[8]);

    /* FMT HEADER */
    wav->sub_chunk1_id    = read_be32(&metadata[12]);
    wav->sub_chunk1_size  = read_le32(&metadata[16]);
    wav->audio_format     = read_le16(&metadata[20]);
    wav->num_channels     = read_le16(&metadata[22]);
    wav->sample_rate      = read_le32(&metadata[24]);
    wav->byte_rate        = read_le32(&metadata[28]);
    wav->block_align      = read_le16(&metadata[32]);
    wav->bit_depth        = read_le16(&metadata[34]);

    /* DATA HEADER */
    wav->sub_chunk2_id    = read_be32(&metadata[36]);
    wav->sub_chunk2_size  = read_le32(&metadata[40]);

    if((wav->sub_chunk2_size) > MAX_FILE_SIZE){
        fprintf(stderr, "File size exceeds maximum allowed size!");
        free(wav);
        fclose(wav_handle);
        return NULL;
    }

    wav->data = malloc(wav->sub_chunk2_size);
    if(!(wav->data)){
        perror("Cannot allocate wav buffer object!");
        free(wav);
        fclose(wav_handle);
        return NULL;
    }

    fseek(wav_handle, 44, SEEK_SET);
    if(wav->sub_chunk2_size != fread(wav->data,1, wav->sub_chunk2_size, wav_handle)){
        fprintf(stderr, "Cannot read audio data!");
        free(wav->data);
        free(wav);
        fclose(wav_handle);
        return NULL;
    }

    fclose(wav_handle);
    return wav;
}

void wav_info(const wav_t* wav){
    if(!wav){ return; }
    char ascii[5];

    printf("--RIFF HEADER-- \n");
    bytes_to_ascii(wav->chunk_id, ascii);
    printf("CHUNK_ID: %s\n", ascii);
    printf("CHUNK_SIZE: %d\n", wav->format);
    bytes_to_ascii(wav->format, ascii);
    printf("FORMAT: %s\n\n", ascii);

    printf("--FMT HEADER-- \n");
    bytes_to_ascii(wav->sub_chunk1_id, ascii);
    printf("SUB_CHUNK1_ID: %s\n", ascii);
    printf("SUB_CHUNK1_SIZE (in bytes): %d\n", (wav->sub_chunk1_size));
    if((wav->audio_format) == 1){
        printf("AUDIO_FORMAT: PCM \n");
    }
    else{
        printf("AUDIO_FORMAT: %d (UNKNOWN) \n", wav->audio_format);
    }
    switch (wav->num_channels){
    case 1:
        printf("NUMBER OF CHANNELS: (mono) \n");
        break;
    case 2:
        printf("NUMBER OF CHANNELS: (stereo) \n");
        break;
    case 3:
        printf("NUMBER OF CHANNELS: (LCR) \n");
        break;
    default:
        printf("NUMBER OF CHANNELS: %d\n", wav->num_channels);
        break;
    }



    printf("SAMPLE RATE: %d\n", wav->sample_rate);
    printf("BYTE RATE: %d\n", wav->byte_rate);
    printf("BLOCK ALIGN: %d\n", wav->block_align);
    printf("BIT DEPTH: %d\n\n", wav->bit_depth);

    printf("--DATA HEADER-- \n");
    bytes_to_ascii(wav->sub_chunk2_id, ascii);
    printf("SUB_CHUNK2_ID: %s\n", ascii);
    printf("DATA SIZE (in KB): %d\n", ((wav->sub_chunk2_size)/1024) );
}


wav_status_t release_wav(wav_t* wav){
    if(!wav){
        return WAV_ERROR_NULL_POINTER;
    }
    if(!(wav->data)){
        return WAV_ERROR_NULL_DATA;
    }
    free(wav->data);
    free(wav);
    return WAV_SUCCESS;
}

/*
wav_status_t export_wav(const wav_t* wav, FILE* out_file){
    // TODO
}
*/