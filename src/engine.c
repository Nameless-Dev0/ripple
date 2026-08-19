#include <stdint.h>
#include <string.h>
#include <math.h>
#include "engine.h"
#include "biquad.h"

#define MAX_ORDER 64 /* For BPF and BSF MAX_ORDER -> 1/2 * MAX_ORDER */
#define MAX_P_Z (2 * MAX_ORDER)
#define RE(c) ((c)[0])
#define IM(c) ((c)[1])
#define PI 3.141592653589793

/*
static void lp2hp(void);
static void lp2bp(void);
static void lp2bs(void);
*/

typedef double complex_t[2];

typedef struct butter_s{
    complex_t zeros[MAX_P_Z];
    complex_t poles[MAX_P_Z];
} butter_t;

static inline  void complex_sub(complex_t result, const complex_t a, const complex_t b){
    RE(result) = RE(a) - RE(b);
    IM(result) = IM(a) - IM(b);
}

static inline  void complex_add(complex_t result, const complex_t a, const complex_t b){
    RE(result) = RE(a) + RE(b);
    IM(result) = IM(a) + IM(b);
}

static inline  void complex_mult(complex_t result, const complex_t a, const complex_t b){
    double re = (RE(a) * RE(b)) - (IM(a) * IM(b));
    double im = (RE(a) * IM(b)) + (IM(a) * RE(b));
    RE(result) = re;
    IM(result) = im;
}

static inline void complex_pow(complex_t result, const complex_t a, uint8_t n){
    if (n == 0){
        RE(result) = 1.0;
        IM(result) = 0.0;
        return;
    }
    if (RE(a) == 0.0 && IM(a) == 0.0){
        RE(result) = 0.0;
        IM(result) = 0.0;
        return;
    }

    RE(result) = 1.0;
    IM(result) = 0.0;
    complex_t base = { RE(a), IM(a) };
    while (n > 0){  /* binary exponentiation */
        if (n & 1){
            complex_mult(result, result, base);
        }
        complex_t sq;
        complex_mult(sq, base, base);
        RE(base) = RE(sq);
        IM(base) = IM(sq);
        n >>= 1;
    }
}

static inline void compute_lpf_butter_poles(butter_t* butter, uint8_t N){
    if(N > MAX_ORDER){
        return;
    }

    double re, im, theta;
    const double step = PI / N;
    for (uint8_t k = 0; k < N; k++){
        theta = (k + 0.5) * step;
        re = -sin(theta);
        im = cos(theta);
        RE(butter->poles[k]) = re;
        IM(butter->poles[k]) = im;
    }
}

static inline void frequency_transformation(butter_t* butter, char type[4], double cutoff){
    if(!butter || !type){
        return;
    }
    if(strncmp(type, "LPF", 4)){
        
    }
    if(strncmp(type, "HPF", 4)){

    }
    butter->poles;
    butter->zeros;
}

static inline void bilinear_transform(biquad_t* biquad, double sampling_period){
    /* TODO */
}

static void init_butter(butter_t* butter, uint8_t order){
    if(!butter){
        return;
    }
    memset(butter->zeros, 0, sizeof(complex_t) * MAX_ORDER);
    memset(butter->poles, 0, sizeof(complex_t) * MAX_ORDER);
    compute_lpf_butter_poles(butter, order);
    
}

void butter(wav_t* wav, uint8_t order, char type[4], float cutoff_freq, float bandwidth){
    init_butter();
}

void lpf_test(wav_t* wav){
    biquad_t q = {
        .b0 = 0.001970,
        .b1 = 0.003940,
        .b2 = 0.001970,
        .a1 = -1.870563,
        .a2 = 0.878444,
        .w1 = {0},
        .w2 = {0}
    };
    
    apply_biquad(wav, &q);
}

void hpf_test(wav_t* wav){
    biquad_t q = {
        .b0 = 0.937252,
        .b1 = -1.874504,
        .b2 = 0.937252,
        .a1 = -1.870563,
        .a2 = 0.878444,
        .w1 = {0},
        .w2 = {0}
    };
    
    apply_biquad(wav, &q);
}

