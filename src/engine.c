#include "engine.h"
#include "biquad.h"

void lpf_test(wav_t* wav){
    /* cutoff at 1024 Hz */

    biquad_t q = {
        .a1 = -1.962975,
        .a2 =  0.963648,
        .b0 =  0.000168,
        .b1 =  0.000336,
        .b2 =  0.000168,
        .w1 = {0},
        .w2 = {0}
    };
    
    apply_biquad(wav, &q);
}

/* 
TODO 
void lpf(wav_t* wav, float cutoff_freq){

}
*/