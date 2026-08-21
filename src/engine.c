#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "wav_internal.h"
#include "engine.h"
#include "biquad.h"

#define MAX_ORDER 64 /* for bpf and bsf max_order <- 1/2 * max_order */
#define MAX_P_Z (2 * MAX_ORDER)
#define MAX_BIQUADS (MAX_P_Z / 2) 

#define RE(c) ((c)[0])
#define IM(c) ((c)[1])
#define PI 3.14159265358979323846

typedef double complex_t[2];

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

static inline double sum_of_products(double a, double b, double c, double d) {
    double w = c * d;
    double e = fma(c, d, -w);
    double f = fma(a, b, w);
    return f + e;
}

static inline double diff_of_products(double a, double b, double c, double d) {
    double w = c * d;
    double e = fma(-c, d, w);
    double f = fma(a, b, -w);
    return f + e;
}

/**
 * This implementation of complex division is based on C99's complex division
 * implementation with a couple of modifications.
 *
 * Reference: Beebe, "Complex Division: C99 Style," in the Mathematical-Function
 * Computation Handbook, Springer, 2017, ch. 15, sec. 15.9, pp. 523–525.
 * doi: 10.1007/978-3-319-64110-2_15
 */

static inline void complex_div(complex_t result, const complex_t a, const complex_t b) {
    double max_b = fmax(fabs(RE(b)), fabs(IM(b)));
    if (max_b == 0.0) {
        RE(result) = RE(a) / RE(b);
        IM(result) = IM(a) / IM(b);
        return;
    }
    int k = ilogb(max_b);

    double br = scalbn(RE(b), -k);
    double bi = scalbn(IM(b), -k);
    double ar = scalbn(RE(a), -k);
    double ai = scalbn(IM(a), -k);

    double denom = sum_of_products(br, br, bi, bi);

    RE(result) = sum_of_products(ar, br, ai, bi) / denom;
    IM(result) = diff_of_products(ai, br, ar, bi) / denom;
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

/* Principal complex square root (branch cut on negative real axis) */
static inline void complex_sqrt(complex_t result, const complex_t a){
    double r = hypot(RE(a), IM(a));
    double re = sqrt(fmax(0.0, (r + RE(a)) / 2.0));
    double im = sqrt(fmax(0.0, (r - RE(a)) / 2.0));
    if (IM(a) < 0.0){
        im = -im;
    }
    RE(result) = re;
    IM(result) = im;
}

typedef struct zpk_s{
    complex_t zeros[MAX_P_Z];
    complex_t poles[MAX_P_Z];
    double gain;
    uint8_t order;
} zpk_t;

static inline void compute_lpf_butter_poles(zpk_t* butter, uint8_t n){
    double re, im, theta_rad;
    const double step = PI / n;
    for (uint8_t k = 0; k < n; k++){
        theta_rad = (k + 0.5) * step;
        re = -sin(theta_rad);
        im = cos(theta_rad);
        RE(butter->poles[k]) = re;
        IM(butter->poles[k]) = im;
    }
}

#define MAX_NYQUIST_PHASE_RATIO (0.4999995) /* maximum phase ratio of pi preventing tan(pi/2) -> inf */
static inline double prewarp_single(double digital_cutoff_hz, double sampling_period) {
    if (sampling_period <= 0.0 || digital_cutoff_hz <= 0.0) {
        return 0.0;
    }
    double angle_rad = PI * digital_cutoff_hz * sampling_period;
    const double max_angle_rad = PI * MAX_NYQUIST_PHASE_RATIO;
    if (angle_rad >= max_angle_rad) {
        angle_rad = max_angle_rad;
    }
    return (2.0 * tan(angle_rad)) / sampling_period;
}

static inline void lp2lp(zpk_t* filter, double cutoff_rad) {
    complex_t cplx_cutoff_rad = {cutoff_rad, 0.0};
    complex_t temp;
    uint8_t zero_count = 0;
    for (uint8_t i = 0; i < filter->order; i++) {
        complex_mult(temp, filter->poles[i], cplx_cutoff_rad);
        RE(filter->poles[i]) = RE(temp);
        IM(filter->poles[i]) = IM(temp);

        if (RE(filter->zeros[i]) != 0.0 || IM(filter->zeros[i]) != 0.0) {
            complex_mult(temp, filter->zeros[i], cplx_cutoff_rad);
            RE(filter->zeros[i]) = RE(temp);
            IM(filter->zeros[i]) = IM(temp);
            zero_count++;
        }
    }
    filter->gain *= pow(cutoff_rad, filter->order - zero_count);
}

static inline void lp2hp(zpk_t* filter, double cutoff_rad) {
    complex_t cplx_cutoff_rad = {cutoff_rad, 0.0};
    complex_t temp, neg;
    complex_t num = {1.0, 0.0};
    complex_t den = {1.0, 0.0};
    for (uint8_t i = 0; i < filter->order; i++) {
        RE(neg) = -RE(filter->poles[i]);
        IM(neg) = -IM(filter->poles[i]);
        complex_mult(den, den, neg);

        if (RE(filter->zeros[i]) != 0.0 || IM(filter->zeros[i]) != 0.0) {
            RE(neg) = -RE(filter->zeros[i]);
            IM(neg) = -IM(filter->zeros[i]);
            complex_mult(num, num, neg);
        }

        complex_div(temp, cplx_cutoff_rad, filter->poles[i]);
        RE(filter->poles[i]) = RE(temp);
        IM(filter->poles[i]) = IM(temp);

        if (RE(filter->zeros[i]) != 0.0 || IM(filter->zeros[i]) != 0.0) {
            complex_div(temp, cplx_cutoff_rad, filter->zeros[i]);
            RE(filter->zeros[i]) = RE(temp);
            IM(filter->zeros[i]) = IM(temp);
        } else {
            RE(filter->zeros[i]) = 0.0;
            IM(filter->zeros[i]) = 0.0;
        }
    }
    complex_t gain_factor;
    complex_div(gain_factor, num, den);
    filter->gain *= RE(gain_factor);
}

/* lowpass -> bandpass, centered at center_rad with bandwidth bw_rad */
static inline void lp2bp(zpk_t* filter, double center_rad, double bw_rad) {
    if (!filter || filter->order == 0) {
        return;
    }
    uint8_t n = filter->order;
    if ((uint16_t)n * 2 > MAX_P_Z) {
        return;
    }
    complex_t poles_lp[MAX_ORDER];
    complex_t zeros_lp[MAX_ORDER];
    uint8_t zero_count = 0;

    complex_t half_bw = {bw_rad / 2.0, 0.0};
    complex_t wo2 = {center_rad * center_rad, 0.0};

    for (uint8_t i = 0; i < n; i++){
        complex_mult(poles_lp[i], filter->poles[i], half_bw);
        if (RE(filter->zeros[i]) != 0.0 || IM(filter->zeros[i]) != 0.0){
            complex_mult(zeros_lp[zero_count], filter->zeros[i], half_bw);
            zero_count++;
        }
    }
    uint8_t degree = n - zero_count;

    for (uint8_t i = 0; i < n; i++){
        complex_t sq, root, plus, minus;
        complex_mult(sq, poles_lp[i], poles_lp[i]);
        complex_sub(sq, sq, wo2);
        complex_sqrt(root, sq);
        complex_add(plus, poles_lp[i], root);
        complex_sub(minus, poles_lp[i], root);
        RE(filter->poles[i]) = RE(plus);
        IM(filter->poles[i]) = IM(plus);
        RE(filter->poles[i + n]) = RE(minus);
        IM(filter->poles[i + n]) = IM(minus);
    }

    for (uint8_t i = 0; i < zero_count; i++){
        complex_t sq, root, plus, minus;
        complex_mult(sq, zeros_lp[i], zeros_lp[i]);
        complex_sub(sq, sq, wo2);
        complex_sqrt(root, sq);
        complex_add(plus, zeros_lp[i], root);
        complex_sub(minus, zeros_lp[i], root);
        RE(filter->zeros[i]) = RE(plus);
        IM(filter->zeros[i]) = IM(plus);
        RE(filter->zeros[i + zero_count]) = RE(minus);
        IM(filter->zeros[i + zero_count]) = IM(minus);
    }
    for (uint8_t i = 0; i < degree; i++){
        RE(filter->zeros[2 * zero_count + i]) = 0.0;
        IM(filter->zeros[2 * zero_count + i]) = 0.0;
    }

    filter->order = 2 * n;
    filter->gain *= pow(bw_rad, degree);
}

static inline void lp2bs(zpk_t* filter, double center_rad, double bw_rad) {
    if (!filter || filter->order == 0) {
        return;
    }
    uint8_t n = filter->order;
    if ((uint16_t)n * 2 > MAX_P_Z) {
        return;
    }
    complex_t poles_hp[MAX_ORDER];
    complex_t zeros_hp[MAX_ORDER];
    uint8_t zero_count = 0;

    complex_t half_bw = {bw_rad / 2.0, 0.0};
    complex_t wo2 = {center_rad * center_rad, 0.0};

    complex_t neg_poles_prod = {1.0, 0.0};
    complex_t neg_zeros_prod = {1.0, 0.0};

    for (uint8_t i = 0; i < n; i++){
        complex_t neg;
        RE(neg) = -RE(filter->poles[i]);
        IM(neg) = -IM(filter->poles[i]);
        complex_mult(neg_poles_prod, neg_poles_prod, neg);

        complex_div(poles_hp[i], half_bw, filter->poles[i]);

        if (RE(filter->zeros[i]) != 0.0 || IM(filter->zeros[i]) != 0.0){
            RE(neg) = -RE(filter->zeros[i]);
            IM(neg) = -IM(filter->zeros[i]);
            complex_mult(neg_zeros_prod, neg_zeros_prod, neg);

            complex_div(zeros_hp[zero_count], half_bw, filter->zeros[i]);
            zero_count++;
        }
    }
    uint8_t degree = n - zero_count;

    for (uint8_t i = 0; i < n; i++){
        complex_t sq, root, plus, minus;
        complex_mult(sq, poles_hp[i], poles_hp[i]);
        complex_sub(sq, sq, wo2);
        complex_sqrt(root, sq);
        complex_add(plus, poles_hp[i], root);
        complex_sub(minus, poles_hp[i], root);
        RE(filter->poles[i]) = RE(plus);
        IM(filter->poles[i]) = IM(plus);
        RE(filter->poles[i + n]) = RE(minus);
        IM(filter->poles[i + n]) = IM(minus);
    }

    for (uint8_t i = 0; i < zero_count; i++){
        complex_t sq, root, plus, minus;
        complex_mult(sq, zeros_hp[i], zeros_hp[i]);
        complex_sub(sq, sq, wo2);
        complex_sqrt(root, sq);
        complex_add(plus, zeros_hp[i], root);
        complex_sub(minus, zeros_hp[i], root);
        RE(filter->zeros[i]) = RE(plus);
        IM(filter->zeros[i]) = IM(plus);
        RE(filter->zeros[i + zero_count]) = RE(minus);
        IM(filter->zeros[i + zero_count]) = IM(minus);
    }
    /* remaining zeros-at-infinity move to +/- j*center_rad, the stopband center */
    for (uint8_t i = 0; i < degree; i++){
        RE(filter->zeros[2 * zero_count + i]) = 0.0;
        IM(filter->zeros[2 * zero_count + i]) = center_rad;
    }
    for (uint8_t i = 0; i < degree; i++){
        RE(filter->zeros[2 * zero_count + degree + i]) = 0.0;
        IM(filter->zeros[2 * zero_count + degree + i]) = -center_rad;
    }

    filter->order = 2 * n;
    complex_t gain_factor;
    complex_div(gain_factor, neg_zeros_prod, neg_poles_prod);
    filter->gain *= RE(gain_factor);
}

static void freq_map(zpk_t* filter, filter_type_t type, double cutoff_freq_hz, double bandwidth_hz, double sampling_period) {
    if (!filter) {
        return;
    }
    switch (type){
    case LPF: {
        double analog_cutoff_rad = prewarp_single(cutoff_freq_hz, sampling_period);
        lp2lp(filter, analog_cutoff_rad);
        break;
    }
    case HPF: {
        double analog_cutoff_rad = prewarp_single(cutoff_freq_hz, sampling_period);
        lp2hp(filter, analog_cutoff_rad);
        break;
    }
    case BPF: {
        double low_hz = cutoff_freq_hz - bandwidth_hz / 2.0;
        double high_hz = cutoff_freq_hz + bandwidth_hz / 2.0;
        double w_low_rad = prewarp_single(low_hz, sampling_period);
        double w_high_rad = prewarp_single(high_hz, sampling_period);
        double bw_rad = w_high_rad - w_low_rad;
        double center_rad = sqrt(w_low_rad * w_high_rad);
        lp2bp(filter, center_rad, bw_rad);
        break;
    }
    case BSF: {
        double low_hz = cutoff_freq_hz - bandwidth_hz / 2.0;
        double high_hz = cutoff_freq_hz + bandwidth_hz / 2.0;
        double w_low_rad = prewarp_single(low_hz, sampling_period);
        double w_high_rad = prewarp_single(high_hz, sampling_period);
        double bw_rad = w_high_rad - w_low_rad;
        double center_rad = sqrt(w_low_rad * w_high_rad);
        lp2bs(filter, center_rad, bw_rad);
        break;
    }
    default:
        break;
    }
}

/* Converts an N-th order analog filter into an equivalent digital filter */
static void bilinear_transform(zpk_t* analog_filter, zpk_t* digital_filter){
    /* todo */
}

/* Converts N-th order digital filter into series of cascaded biquads */
static void df2sos(zpk_t* digital_filter, bq_df_t* biquads){
    /* todo */
}

/* Applies series of cascaded biquads to audio samples (using TDFII) */
static void apply_biquads_tdf2(wav_t* wav, const bq_df_t* biquads){
    /* todo */
}

void filter(wav_t* wav, uint8_t order, filter_type_t type, float cutoff_freq_hz, float bandwidth_hz){
    zpk_t a_butter;
    a_butter.order = order;
    memset(&(a_butter.zeros), 0, sizeof(complex_t) * MAX_P_Z);
    memset(&(a_butter.poles), 0, sizeof(complex_t) * MAX_P_Z);
    compute_lpf_butter_poles(&a_butter, order);
    a_butter.gain = 1.0;

    double sampling_period = 1.0/(wav->sample_rate);
    freq_map(&a_butter, type, cutoff_freq_hz, bandwidth_hz, sampling_period);

    zpk_t d_butter;
    bilinear_transform(&a_butter, &d_butter);
    
    bq_df_t biquads[MAX_BIQUADS];
    df2sos(&d_butter, biquads);

    apply_biquads_tdf2(wav, biquads);
}