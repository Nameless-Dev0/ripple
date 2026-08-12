#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include "wav.h"

static inline uint16_t read_le16(const uint8_t* p);  /* Converts host 16-bit integer to little endian    */
static inline uint32_t read_le32(const uint8_t* p);  /* Converts host 32-bit integer to little endian */
static inline void write_le32(const uint8_t *p, FILE* file);  /* Writes to 16-bit integer to file in little endian */ 
static inline void write_le16(const uint8_t *p, FILE* file);  /* Writes to 16-bit integer to file in little endian */ 

uint32_t read_le32(const uint8_t* p) {
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        uint32_t val;
        memcpy(&val, p, sizeof(val));
        return val;
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        return  (uint32_t)p[0]        | ((uint32_t)p[1] << 8) |
            ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
    #endif
}

uint16_t read_le16(const uint8_t* p) {
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        uint16_t val;
        memcpy(&val, p, sizeof(val));
        return val;
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
#endif
}

void write_le16(const uint8_t* p, FILE* file) {
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        fwrite((const uint16_t*)p, 1, sizeof(uint16_t), file);
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        uint16_t le16 = (uint16_t)p[0] | ((uint16_t)p[1] << 8);
        fwrite(&le16, 1, sizeof(uint16_t), file);
    #endif
}

void write_le32(const uint8_t* p, FILE* file) {
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        fwrite((const uint32_t*)p, 1, sizeof(uint32_t), file);
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        uint32_t le32 = (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
        fwrite(&le32, 1, sizeof(uint32_t), file);
    #endif
}

typedef struct wav_s{
    uint16_t durartion; /* In seconds */
    
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


wav_t* load_wav(const char* wav_file){
    if(!wav_file){
        fprintf(stderr, "Failed to load... file does not exist!");
        return NULL;
    }
    FILE* wav_handle = fopen(wav_file, "rb");
    if(!wav_handle){
        perror("Cannot open file!");
        return NULL;
    }

    wav_t* wav = malloc(sizeof(struct wav_s));
    if(!wav){
        perror("Cannot allocate wav object!");
        if(fclose(wav_handle) == EOF){
            perror("Load failed to close file!");
        }
        return NULL;
    }

    uint8_t metadata[44];
    if(sizeof(metadata) != fread(metadata,1, sizeof(metadata), wav_handle)){
        perror("Unable to extract WAV metadata!");
        free(wav);
        if(fclose(wav_handle) == EOF){
            perror("Load failed to close file!");
        }
        return NULL;
    }

    /* RIFF HEADER */
    memmove(wav->chunk_id, &metadata[0],  4);
    wav->chunk_size = read_le32(&metadata[4]);
    memmove(wav->format, &metadata[8],  4);

    /* FMT HEADER */
    memmove(wav->sub_chunk1_id,  &metadata[12], 4);
    wav->sub_chunk1_size= read_le32(&metadata[16]);
    wav->audio_format=    read_le16(&metadata[20]);
    wav->num_channels=    read_le16(&metadata[22]);
    wav->sample_rate=     read_le32(&metadata[24]);
    wav->byte_rate=       read_le32(&metadata[28]);
    wav->block_align=     read_le16(&metadata[32]);
    wav->bit_depth=       read_le16(&metadata[34]);

    /* DATA HEADER */
    memmove(wav->sub_chunk2_id, &metadata[36], 4);
    wav->sub_chunk2_size = read_le32(&metadata[40]);

    wav->chunk_id[4] = '\0';
    wav->format[4] = '\0';
    wav->sub_chunk1_id[4] = '\0';
    wav->sub_chunk2_id[4] = '\0';

    if(is_valid_wav(wav) == false){
        free(wav);
        if(fclose(wav_handle) == EOF){
            perror("Load failed to close file!");
        }
        fprintf(stderr, "Failed to load... invalid WAV file!");
        return NULL;
    }

    if((wav->sub_chunk2_size) > MAX_FILE_SIZE){
        fprintf(stderr, "File size exceeds maximum allowed size!");
        free(wav);
        if(fclose(wav_handle) == EOF){
            perror("Load failed to close file!");
        }
        return NULL;
    }

    wav->data = NULL;
    if(wav->sub_chunk2_size != 0 && !(wav->data = malloc(wav->sub_chunk2_size))){
        perror("Cannot allocate wav buffer object!");
        free(wav);
        if(fclose(wav_handle) == EOF){
            perror("Load failed to close file!");
        }
        return NULL;
    }


    fseek(wav_handle, 44, SEEK_SET);
    if(wav->sub_chunk2_size != fread(wav->data,1, wav->sub_chunk2_size, wav_handle)){
        fprintf(stderr, "Cannot read audio data!");
        free(wav->data);
        free(wav);
        if(fclose(wav_handle) == EOF){
            perror("Load failed to close file!");
        }
        return NULL;
    }

    wav->durartion = (wav->sub_chunk2_size)/(wav->byte_rate);

    if(fclose(wav_handle) == EOF){
        perror("Load failed to close file!");
        release_wav(wav);
        return NULL;
    }

    return wav;
}

/* Note: A WAV file with valid metadata but no acutal data is still valid */
bool is_valid_wav(const wav_t* wav){
    if (wav == NULL) {
        return false;
    }
    if(wav->audio_format != 1 || wav->sub_chunk1_size != 16){
        return false;
    }
    if(wav->num_channels == 0 || wav->num_channels > 256 ||
       wav->bit_depth == 0    || wav->bit_depth > 64     ||
       wav->sample_rate == 0 || (wav->bit_depth % 8 != 0)) {
        return false;
    }
    if((memcmp(wav->chunk_id, "RIFF", 4) != 0)      ||
       (memcmp(wav->format, "WAVE", 4)   != 0)      ||
       (memcmp(wav->sub_chunk1_id, "fmt ", 4) != 0) ||
       (memcmp(wav->sub_chunk2_id, "data", 4) != 0)){
        return false;
    }
    uint64_t expected_block_align = ((uint64_t)wav->num_channels * wav->bit_depth) / 8;
    uint64_t expected_byte_rate   = (uint64_t)wav->sample_rate * expected_block_align;

    if (wav->block_align != expected_block_align ||
        wav->byte_rate != expected_byte_rate) {
        return false;
    }

    return true;
}

void wav_info(const wav_t* wav) {
    if (!wav){ 
        return;
    }

    printf("--RIFF HEADER-- \n");
    printf("CHUNK_ID: %s\n", wav->chunk_id);
    printf("CHUNK_SIZE: %u\n", wav->chunk_size);
    printf("FORMAT: %s\n\n", wav->format);

    printf("--FMT HEADER-- \n");
    printf("SUB_CHUNK1_ID: %s\n", wav->sub_chunk1_id);
    printf("SUB_CHUNK1_SIZE (in bytes): %u\n", wav->sub_chunk1_size);
    if (wav->audio_format == 1) {
        printf("AUDIO_FORMAT: PCM \n");
    } else {
        printf("AUDIO_FORMAT: %u (UNKNOWN) \n", wav->audio_format);
    }

    switch (wav->num_channels) {
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
        printf("NUMBER OF CHANNELS: %u\n", wav->num_channels);
        break;
    }

    printf("SAMPLE RATE: %u\n", wav->sample_rate);
    printf("BYTE RATE: %u\n", wav->byte_rate);
    printf("BLOCK ALIGN: %u\n", wav->block_align);
    printf("BIT DEPTH: %u\n\n", wav->bit_depth);

    printf("--DATA HEADER-- \n");
    printf("SUB_CHUNK2_ID: %s\n", wav->sub_chunk2_id);
    printf("DATA SIZE: %.2f MB\n", (wav->sub_chunk2_size)/(float)(1024*1024));
    printf("DURATION: %d s\n\n", (wav->durartion));
}

wav_status_t release_wav(wav_t* wav){
    if(!wav){
        return WAV_NULL_POINTER;
    }
    if(!(wav->data)){
        free(wav);
        fprintf(stderr, "[WARNING] freed wav file has no data!");
        return WAV_SUCCESS;
    }

    free(wav->data);
    free(wav);
    return WAV_SUCCESS;
}

wav_status_t export_wav(const wav_t* wav, const char* file_path){
    if(!wav){
        return WAV_NULL_POINTER;
    }
    if(!(file_path)){
        return WAV_INVALID_EXPORT_PATH;
    }

    FILE* out = fopen(file_path, "wb");
    if(!out){
        return WAV_EXPORT_ERR;
    }

    /* RIFF HEADER */
    fwrite(&(wav->chunk_id),    sizeof(wav->chunk_id)-1, 1, out);
    write_le32((uint8_t*)&(wav->chunk_size), out);
    fwrite(&(wav->format),      sizeof(wav->format)-1, 1, out);

    /* FMT HEADER */
    fwrite(&(wav->sub_chunk1_id),   sizeof(wav->sub_chunk1_id)-1, 1, out);
    write_le32((uint8_t*)&(wav->sub_chunk1_size), out);
    write_le16((uint8_t*)&(wav->audio_format), out);
    write_le16((uint8_t*)&(wav->num_channels), out);
    write_le32((uint8_t*)&(wav->sample_rate), out);
    write_le32((uint8_t*)&(wav->byte_rate), out);
    write_le16((uint8_t*)&(wav->block_align), out);
    write_le16((uint8_t*)&(wav->bit_depth), out);

    /* DATA HEADER */
    fwrite(&(wav->sub_chunk2_id),   sizeof(wav->sub_chunk2_id)-1, 1, out);
    write_le32((uint8_t*)&(wav->sub_chunk2_size), out);
    if(!(wav->data)){
        fprintf(stderr, "[WARNING] exporting file with no audio data!");
    }
    else{
        fwrite(wav->data, 1, wav->sub_chunk2_size, out);
    }

    if(fclose(out) == EOF){
        perror("Export failed to close output file!");
        return WAV_EXPORT_ERR;
    }
    return WAV_SUCCESS;
}