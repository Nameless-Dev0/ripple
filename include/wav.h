#ifndef WAV_H
#define WAV_H

#include <stdio.h>
#define MAX_FILE_SIZE 524288000 /* 500 MB */

typedef struct wav_s wav_t;

typedef enum {
    WAV_SUCCESS = 0,
    WAV_ERR_NULL_POINTER,
    WAV_ERR_NULL_DATA,
    WAV_EXPORT_ERR
} wav_status_t;

wav_t* load_wav(const char* wav_file);    /* Loads a WAV file and returns a handle to a WAV struct */
void wav_info(const wav_t* wav);          /* Displays metadata information of the passed wav       */
wav_status_t release_wav(wav_t* wav);     /* Frees wav allocated buffers                           */

/** 
 * Serializes a wav struct out to a WAV-formatted file. 
 * The engine does not modify the .wav file in any way that requires recomputing 
 * the file's metadata, i.e., the export file reuses the original file's metadata. 
*/
wav_status_t export_wav(const wav_t* wav);                 

#endif /* WAV_H */