#ifndef WAV_H
#define WAV_H
#include <stdio.h>

typedef struct wav_s wav_t;

/* Loads a WAV file and returns a handle to a WAV struct*/
wav_t* load_wav(FILE* wav_file)

/*  */
void read_wav(wav_t* wav)

/* Frees wav allocated buffers and closes wav_file */
void release_wav(wav_t* wav)

/*  */
FILE* export_wav(FILE* wav_file)

#endif /* WAV_H */ 