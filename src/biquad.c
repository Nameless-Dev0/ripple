#include <stdint.h>
#include "biquad.h"
#include "wav.h"
#include "wav_internal.h"


#define PROCESS_BUFFER_MONO(TYPE, WAV, BIQUAD)                   \
    do {                                                         \
        TYPE* x = (TYPE*)(WAV->data);                            \
        size_t count = (WAV->sub_chunk2_size)/sizeof(TYPE);      \
                                                                 \
        for (size_t i = 0; i < count; i++) {                                     \
            double x_i = (double)x[i];                                           \
            double y_i = BIQUAD->b0 * x_i + BIQUAD->w1[0];                       \
            BIQUAD->w1[0] = BIQUAD->b1 * x_i - BIQUAD->a1 * y_i + BIQUAD->w2[0]; \
            BIQUAD->w2[0] = BIQUAD->b2 * x_i - BIQUAD->a2 * y_i;                 \
            x[i] = (TYPE)y_i;                                                    \
        }                                                                        \
    } while(0)


#define PROCESS_BUFFER_STEREO(TYPE, WAV, BIQUAD)                 \
    do {                                                         \
        TYPE* x = (TYPE*)(WAV->data);                            \
        size_t channels = WAV->num_channels;                     \
        size_t count = (WAV->sub_chunk2_size)/sizeof(TYPE);      \
                                                                 \
        for (size_t i = 0; i < count; i+=channels) {                                      \
            double x_ch0_i = (double)x[i];                                                \
            double x_ch1_i = (double)x[i+1];                                              \
                                                                                          \
            double y_ch0_i = BIQUAD->b0 * x_ch0_i + BIQUAD->w1[0];                        \
            double y_ch1_i = BIQUAD->b0 * x_ch1_i + BIQUAD->w1[1];                        \
                                                                                          \
            BIQUAD->w1[0] = BIQUAD->b1 * x_ch0_i - BIQUAD->a1 * y_ch0_i + BIQUAD->w2[0];  \
            BIQUAD->w1[1] = BIQUAD->b1 * x_ch1_i - BIQUAD->a1 * y_ch1_i + BIQUAD->w2[1];  \
                                                                                          \
            BIQUAD->w2[0] = BIQUAD->b2 * x_ch0_i - BIQUAD->a2 * y_ch0_i;        \
            BIQUAD->w2[1] = BIQUAD->b2 * x_ch1_i - BIQUAD->a2 * y_ch1_i;        \
                                                                                \
            x[i] = (TYPE)y_ch0_i;                                               \
            x[i+1] = (TYPE)y_ch1_i;                                             \
        }                                                                       \
    } while(0)

void apply_biquad(wav_t* wav, biquad_t* biquad) {
    if(wav->num_channels == 1){
        switch (wav->bit_depth) {
            case 8:  PROCESS_BUFFER_MONO(uint8_t,  wav, biquad); break;
            case 16: PROCESS_BUFFER_MONO(int16_t, wav, biquad); break;
            case 32: PROCESS_BUFFER_MONO(int32_t, wav, biquad); break;
            default: break;
        }
    }

    else if(wav->num_channels == 2){
        switch (wav->bit_depth) {
            case 8:  PROCESS_BUFFER_STEREO(uint8_t,  wav, biquad); break;
            case 16: PROCESS_BUFFER_STEREO(int16_t, wav, biquad); break;
            case 32: PROCESS_BUFFER_STEREO(int32_t, wav, biquad); break;
            default: break;
        }
    }

    else{
        return;
    }
}