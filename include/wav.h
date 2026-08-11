#ifndef WAV_H
#define WAV_H
#include <stdio.h>

#define MAX_FILE_SIZE 524288000 /* 500 MB */

typedef struct wav_s wav_t;

typedef enum {
    WAV_SUCCESS = 0,
    WAV_ERROR_NULL_POINTER,
    WAV_ERROR_NULL_DATA,
    WAV_ERROR_INVALID_HEADER,
} wav_status_t;

wav_t* load_wav(const char* wav_file);                     /* Loads a WAV file and returns a handle to a WAV struct */
void wav_info(const wav_t* wav);                                 /* Displays metadata information of the passed wav       */
wav_status_t release_wav(wav_t* wav);                      /* Frees wav allocated buffers                           */
wav_status_t export_wav(const wav_t* wav, FILE* out_file); /* Serializes a wav struct out to a WAV-formatted file.  */

#endif /* WAV_H */