#ifndef BIQUAD_H
#define BIQUAD_H
#include "wav.h"

#define MAX_CHANNELS 2

typedef  struct biquad_s{
    double a1, a2;
    double b0, b1, b2;
    double w1[MAX_CHANNELS], w2[MAX_CHANNELS]; /* state registers for each channel */
} biquad_t;

void apply_biquad(wav_t* wav, biquad_t* biquad);

#endif /* BIQUAD_H */