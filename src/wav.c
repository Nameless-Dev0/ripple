#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include "wav.h"
#include "wav_internal.h"

void get_err_msg(wav_status_t status, char* dest_buf, size_t dest_size){
    if(!dest_buf || dest_size < WAV_ERR_MSG_MIN){
        return;
    }

    const char* msg;
    switch (status){
        case WAV_SUCCESS: msg = "Success"; break;
        case WAV_NULL_POINTER: msg = "Null pointer passed"; break;
        case WAV_NULL_DATA: msg = "WAV has no audio data"; break;
        case WAV_INVALID_PATH_ERR: msg = "Invalid or empty file path"; break;
        case WAV_FILE_OPEN_ERR: msg = "Failed to open file"; break;
        case WAV_FILE_READ_ERR: msg = "Failed to read file"; break;
        case WAV_INVALID_FORMAT: msg = "Invalid or malformed WAV format"; break;
        case WAV_FILE_TOO_LARGE: msg = "File exceeds maximum allowed size"; break;
        case WAV_ALLOC_ERR: msg = "Memory allocation failed"; break;
        case WAV_INVALID_EXPORT_PATH: msg = "Invalid export file path"; break;
        case WAV_EXPORT_ERR: msg = "Failed to export WAV file"; break;
        case WAV_FILE_CLOSE_ERR: msg = "Failed to close file"; break;
        default: msg = "Unknown error"; break;
    }

    snprintf(dest_buf, dest_size, "%s", msg);
}

static inline void set_status(wav_status_t* out_status, wav_status_t status){
    if(out_status){
        *out_status = status;
    }
}

wav_t* load_wav(const char* wav_file, wav_status_t* status){
    if(!wav_file){
        set_status(status, WAV_INVALID_PATH_ERR);
        return NULL;
    }
    FILE* wav_handle = fopen(wav_file, "rb");
    if(!wav_handle){
        set_status(status, WAV_FILE_OPEN_ERR);
        return NULL;
    }
    wav_t* wav = malloc(sizeof(struct wav_s));
    if(!wav){
        set_status(status, WAV_ALLOC_ERR);
        fclose(wav_handle);
        return NULL;
    }
    uint8_t metadata[44];
    if(sizeof(metadata) != fread(metadata,1, sizeof(metadata), wav_handle)){
        set_status(status, WAV_FILE_READ_ERR);
        free(wav);
        fclose(wav_handle);
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
        set_status(status, WAV_INVALID_FORMAT);
        free(wav);
        fclose(wav_handle);
        return NULL;
    }
    if((wav->sub_chunk2_size) > MAX_FILE_SIZE){
        set_status(status, WAV_FILE_TOO_LARGE);
        free(wav);
        fclose(wav_handle);
        return NULL;
    }
    wav->data = NULL;
    if(wav->sub_chunk2_size != 0 && !(wav->data = malloc(wav->sub_chunk2_size))){
        set_status(status, WAV_ALLOC_ERR);
        free(wav);
        fclose(wav_handle);
        return NULL;
    }
    if(fseek(wav_handle, sizeof(metadata), SEEK_SET) != 0){
        set_status(status, WAV_FILE_READ_ERR);
        free(wav->data);
        free(wav);
        fclose(wav_handle);
        return NULL;
    }
    if(wav->sub_chunk2_size > 0 && fread(wav->data,1, wav->sub_chunk2_size, wav_handle) != wav->sub_chunk2_size){
        set_status(status, WAV_FILE_READ_ERR);
        free(wav->data);
        free(wav);
        fclose(wav_handle);
        return NULL;
    }
    wav->duration = (wav->sub_chunk2_size)/(wav->byte_rate);
    if(fclose(wav_handle) == EOF){
        set_status(status, WAV_FILE_CLOSE_ERR);
        release_wav(wav);
        return NULL;
    }
    set_status(status, WAV_SUCCESS);
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
    printf("DURATION: %u s\n\n", (wav->duration));
}

wav_status_t release_wav(wav_t* wav){
    if(!wav){
        return WAV_NULL_POINTER;
    }
    if(!(wav->data)){
        free(wav);
        return WAV_NULL_DATA;
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