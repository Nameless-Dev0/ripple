#ifndef BIQUAD_H
#define BIQUAD_H

#include "wav.h"
#define MAX_CHANNELS 2

/*
    b0 + (b1)z^-1 + (b2)z^-2
    ------------------------
    1 + (a1)z^-1 + (a2)z^-2
*/
typedef struct bq_df_s{
    double a1, a2;
    double b0, b1, b2;
    double w1[MAX_CHANNELS], w2[MAX_CHANNELS]; /* state registers for each channel */
} bq_df_t;


#endif /* BIQUAD_H */