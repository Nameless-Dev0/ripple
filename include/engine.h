#ifndef ENGINE_H
#define ENGINE_H
#include <stdio.h>
#include <stdint.h>

#ifndef TARGET_NORM_RMS
    #define TARGET_NORM_RMS -14 /* Change preceived loudness (dBFS) for normalization */ 
#endif

typedef struct wav_s wav_t; /* struct containing parsed wav file pointed to by a FILE* handle */

typedef enum {
    LPF,
    HPF,
    BPF,
    BSF
} filter_type_t;

/**
 * Applies an IIR filter based on a butterworth prototype
 * Filter types: LPF, HPF, BPF, BSF
 * set bandwidth = 0.0 for LPF & HPF
*/ 
void filter(wav_t* wav, uint8_t order, filter_type_t type, float cutoff_freq_hz, float bandwidth_hz);

void gain(wav_t* wav, float gain_dB);    /* Applys constant gain (in dB) to entire audio signal */
void normalize_audio(wav_t* wav);        /* Applys constant gain (in dB) based on RMS (i.e., preceived loudness) */


#endif /* ENGINE_H */