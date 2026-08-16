#ifndef WAV_H
#define WAV_H

#include <stddef.h>
#include <stdbool.h>

#define MAX_FILE_SIZE 524288000 /* 500 MB */
#define WAV_ERR_MSG_MAX 128
#define WAV_ERR_MSG_MIN 32

typedef struct wav_s wav_t;

typedef enum {
    WAV_SUCCESS = 0,
    WAV_NULL_POINTER,
    WAV_NULL_DATA,
    WAV_INVALID_PATH_ERR,
    WAV_FILE_OPEN_ERR,
    WAV_FILE_CLOSE_ERR,
    WAV_FILE_READ_ERR,
    WAV_INVALID_FORMAT,
    WAV_FILE_TOO_LARGE,
    WAV_ALLOC_ERR,
    WAV_INVALID_EXPORT_PATH,
    WAV_EXPORT_ERR
} wav_status_t;


/* Loads a WAV file and returns a handle to a WAV struct. Set status to NULL to ignore error messages */
wav_t* load_wav(const char* wav_file, wav_status_t* status);

/* Frees wav allocated buffers */
wav_status_t release_wav(wav_t* wav);

/** 
 * Serializes a wav struct out to a WAV-formatted file, with derised filepath (e.g., audio/test.wav). 
 * The engine does not modify the .wav file in any way that requires recomputing 
 * the file's metadata, i.e., the export file reuses the original file's metadata. 
*/
wav_status_t export_wav(const wav_t* wav, const char* file_path);

/* Converts wav error status code to error message */
void get_err_msg(wav_status_t status, char* dest_err_msg, size_t msg_size); 

/* Checks if parsed file is a valid WAV file */
bool is_valid_wav(const wav_t* wav);      

/* Displays metadata information of the passed wav */
void wav_info(const wav_t* wav);          


#endif /* WAV_H */