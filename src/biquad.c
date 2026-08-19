#include <stdint.h>
#include <math.h>
#include <limits.h>
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

#define COMPUTE_AVG_POWER(TYPE, WAV, AVG_PWR)                    \
    do {                                                         \
        TYPE* x = (TYPE*)(WAV->data);                            \
        size_t count = (WAV->sub_chunk2_size)/sizeof(TYPE);      \
        double sum = 0.0;                                        \
        for (size_t i = 0; i < count; i++) {                     \
            double x_i = (double)x[i];                           \
            sum += x_i * x_i;                                    \
        }                                                        \
        (AVG_PWR) = count > 0 ? (sum / (double)count) : 0.0;  \
    } while(0)

#define APPLY_GAIN(TYPE, WAV, GAIN, TMIN, TMAX)                  \
    do {                                                         \
        TYPE* x = (TYPE*)(WAV->data);                            \
        size_t count = (WAV->sub_chunk2_size)/sizeof(TYPE);      \
        for (size_t i = 0; i < count; i++) {                     \
            double y = (double)x[i] * (GAIN);                    \
            if (y > (double)(TMAX)) y = (double)(TMAX);          \
            if (y < (double)(TMIN)) y = (double)(TMIN);          \
            x[i] = (TYPE)y;                                      \
        }                                                        \
    } while(0)

void apply_biquad(wav_t* wav, biquad_t* biquad) {
    if(!wav || !biquad){
        return;
    }

    if(wav->num_channels > 2){
        return;
    }

    double power_in = 0.0;
        switch (wav->bit_depth) {
        case 8:  COMPUTE_AVG_POWER(uint8_t, wav, power_in); break;
        case 16: COMPUTE_AVG_POWER(int16_t, wav, power_in); break;
        case 32: COMPUTE_AVG_POWER(int32_t, wav, power_in); break;
        default: return;
    }

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

    double power_out = 0.0;
    switch (wav->bit_depth) {
        case 8:  COMPUTE_AVG_POWER(uint8_t, wav, power_out); break;
        case 16: COMPUTE_AVG_POWER(int16_t, wav, power_out); break;
        case 32: COMPUTE_AVG_POWER(int32_t, wav, power_out); break;
        default: return;
    }
    if (power_out > 1e-12 && power_in > 1e-12) {
        double gain = sqrt(power_in / power_out);
        switch (wav->bit_depth) {
            case 8:  APPLY_GAIN(uint8_t,  wav, gain, 0, 255); break;
            case 16: APPLY_GAIN(int16_t, wav, gain, INT16_MIN, INT16_MAX); break;
            case 32: APPLY_GAIN(int32_t, wav, gain, INT32_MIN, INT32_MAX); break;
            default: break;
        }
    }
}