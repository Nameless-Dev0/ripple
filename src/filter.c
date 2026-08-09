
#ifndef FILTER_H
#define FILTER_H
#include <stdio.h>

/* struct containing parsed wav file pointed to by a FILE* handle */
typedef struct wav_s wav_t;

/* Apply constant positive (+) gain (in dB) to entire audio signal */
void gain(wav_t);

/* Apply constant negative (-) gain (in dB) to entire audio signal */
void attenuate(wav_t);

/* Pass only frequencies (in Hz) up to cutoff_freq */
void lpf(wav_t, float cutoff_freq);

/* Pass only frequencies (in Hz) above cutoff_freq */
void hpf(wav_t, float cutoff_freq);

#endif /* FILTER_H */