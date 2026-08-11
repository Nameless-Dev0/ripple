#ifndef WAV_H
#define WAV_H
#include <stdio.h>

typedef struct wav_s wav_t;

typedef enum {
    WAV_SUCCESS = 0,
    WAV_ERROR_NULL_POINTER,
    WAV_ERROR_INVALID_HEADER,
    WAV_ERROR_IO,
    WAV_ERROR_BUFFER_TOO_SMALL
} wav_status_t;

wav_t* load_wav(FILE* wav_file);                           /* Loads a WAV file and returns a handle to a WAV struct */
wav_status_t read_wav(wav_t* wav);                         /* Extracts audio sample data from parsed WAV file       */
wav_status_t release_wav(wav_t* wav);                              /* Frees wav allocated buffers and closes wav_file       */
wav_status_t export_wav(const wav_t* wav, FILE* out_file); /* Serializes a wav struct out to a WAV-formatted file.  */

#endif /* WAV_H */