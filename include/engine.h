#ifndef ENGINE_H
#define ENGINE_H
#include <stdio.h>

#ifndef TARGET_NORM_RMS
    #define TARGET_NORM_RMS -14 /* Change preceived loudness (dBFS) for normalization */ 
#endif

typedef struct wav_s wav_t; /* struct containing parsed wav file pointed to by a FILE* handle */

void lpf_test(wav_t* wav); /* simple biquad functionality test */

void gain(wav_t* wav, float gain_dB);    /* Applys constant gain (in dB) to entire audio signal */
void normalize_audio(wav_t* wav);        /* Applys constant gain (in dB) based on RMS (i.e., preceived loudness) */
void lpf(wav_t* wav, float cutoff_freq); /* Cuts off frequencies (in Hz) above cutoff_freq */
void hpf(wav_t* wav, float cutoff_freq); /* Cuts off frequencies (in Hz) below cutoff_freq */

#endif /* ENGINE_H */