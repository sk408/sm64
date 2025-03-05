/**
 * @file g722_encode.c
 * @brief G.722 encoder implementation
 */

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <memory.h>
#include "g722_codec.h"

#define SIGN_BIT        (0x80)
#define QUANT_MASK      (0xf)
#define NSEGS           (8)
#define SEG_SHIFT       (4)
#define SEG_MASK        (0x70)

#define BIAS            (0x84)
#define CLIP            (32635)
#define DLI_OFFSET      (-32)
#define DHI_OFFSET      (0)
#define DLX_OFFSET      (0)
#define DHX_OFFSET      (0)

static const int16_t qm4[16] = {
    0, -20456, -12896, -8968,
    -6288, -4240, -2584, -1200,
    20456, 12896, 8968, 6288,
    4240, 2584, 1200, 0
};

static const int16_t ilb[32] = {
    2048, 2093, 2139, 2186, 2233, 2282, 2332, 2383,
    2435, 2489, 2543, 2599, 2656, 2714, 2774, 2834,
    2896, 2960, 3025, 3091, 3158, 3228, 3298, 3371,
    3444, 3520, 3597, 3676, 3756, 3838, 3922, 4008
};

struct g722_encode_state_s {
    int yl;
    int yu;
    int dms;
    int dml;
    int ap;
    int a[2];
    int b[6];
    int td;
    int sr[2];
    int dq[6];
    int pk[2][3];
    int rate;
    int shift_bits;
};

static int16_t saturate(int32_t amp) {
    int16_t amp16;
    
    /* Saturate to 16 bits */
    if (amp > INT16_MAX)
        amp16 = INT16_MAX;
    else if (amp < INT16_MIN)
        amp16 = INT16_MIN;
    else
        amp16 = (int16_t) amp;
    
    return amp16;
}

static int16_t seg_lookup(const int16_t val) {
    int16_t r;
    int16_t seg;
    int16_t uval = (int16_t) ((val < 0) ? -val : val);
    
    if (uval <= 1)
        return 0;
    if (uval <= 31)
        return 1;
    for (seg = 2; seg < 8; seg++) {
        if (uval <= (1 << seg))
            return seg;
    }
    return 8;
}

static int16_t predictor_pole(int16_t val, int16_t *a) {
    return saturate((a[0] * val) >> 15);
}

static int16_t predictor_zero(int16_t *b, int16_t *dq) {
    int32_t sum;
    
    sum = (b[0] * dq[0]) >> 15;
    sum += (b[1] * dq[1]) >> 15;
    sum += (b[2] * dq[2]) >> 15;
    sum += (b[3] * dq[3]) >> 15;
    sum += (b[4] * dq[4]) >> 15;
    sum += (b[5] * dq[5]) >> 15;
    
    return saturate(sum);
}

static int16_t quantize(int16_t d, int16_t y, int16_t *table, int quantizer) {
    int16_t dqm;
    int16_t dql;
    int16_t dex;
    int16_t dqt;
    int16_t dl;
    
    dqm = (d >= 0) ? d : -(d + 1);
    
    /* Quantize using the appropriate quantizer */
    dex = (dqm >> quantizer) + ((dqm >> (quantizer - 1)) & 1);
    if (dex > 7)
        dex = 7;
    
    dql = dex << quantizer;
    
    dqt = (dql + 2) >> 2;
    
    if (d >= 0)
        dl = table[dqt];
    else
        dl = -table[dqt + 1];
    
    return dl;
}

static int16_t reconstruct(int16_t sign, int16_t dqln, int16_t y) {
    int16_t dql;
    int16_t dex;
    int16_t dqt;
    int16_t dq;
    
    dql = dqln >> 2;
    dex = (y >> 13) & 1;
    dqt = dql + (dex << 7);
    dq = (dqt << 7) + (1 << 6);
    
    return (sign == 0) ? dq : -dq;
}

static int16_t step_size(int16_t y) {
    int16_t dif;
    int16_t al;
    
    if (y > 1535)
        return 2048;
    
    dif = y >> 6;
    al = 0;
    
    while (dif > 0) {
        dif >>= 1;
        al++;
    }
    
    return ilb[al];
}

g722_encode_state_t *g722_encoder_init(int rate, int options) {
    g722_encode_state_t *s;
    
    if ((s = (g722_encode_state_t *) malloc(sizeof(*s))) == NULL)
        return NULL;
    
    memset(s, 0, sizeof(*s));
    
    s->rate = rate;
    s->yl = 34816;
    s->yu = 544;
    
    if (options & 1)
        s->shift_bits = 0;
    else
        s->shift_bits = (rate == 48000) ? 1 : 0;
    
    return s;
}

void g722_encoder_release(g722_encode_state_t *s) {
    free(s);
}

int g722_encode(g722_encode_state_t *s, uint8_t g722_data[], const int16_t pcm_data[], int len) {
    int16_t xh;
    int16_t xl;
    int16_t sh;
    int16_t sl;
    int16_t eh;
    int16_t el;
    int16_t dh;
    int16_t dl;
    int16_t ss;
    int16_t szl;
    int16_t szh;
    int16_t wd1;
    int16_t wd2;
    int16_t wd3;
    int16_t sp;
    int16_t slint;
    int16_t sph;
    int16_t ph;
    int16_t sl1;
    int16_t shl;
    int16_t i;
    int16_t tr;
    int16_t trsv;
    int16_t d;
    int16_t rl42;
    int16_t rh1;
    int16_t rh2;
    int16_t qh;
    int16_t ql;
    int16_t ih;
    int16_t il;
    int k;
    
    for (k = 0; k < len; k++) {
        /* Get the 16-bit input sample */
        wd1 = pcm_data[k];
        wd2 = wd1 >> 8;
        wd3 = (wd1 & 0xFF) << 8;
        
        /* Split into high and low bands */
        xh = wd2;
        xl = wd3;
        
        /* High band predicting filter */
        sh = predictor_pole(s->a[1], s->sr + 1);
        
        /* Add the estimated signal to the quantized difference signal */
        wd1 = saturate(sh + s->dq[0]);
        
        /* Quantizer for high band */
        eh = xh - wd1;
        
        s->dq[0] = quantize(eh, s->yu, qm4, 10);
        
        /* Reconstructed signal */
        rh1 = s->a[1] >> 9;
        rh2 = (s->dq[0] << 2) - rh1;
        s->sr[1] = saturate(rh2);
        
        /* Update high band predictor coefficients */
        s->a[1] = s->a[1] + ((s->dq[0] * 11) >> 7);
        
        /* Low band predicting filter */
        sl = predictor_pole(s->a[0], s->sr);
        
        wd1 = sl + s->dq[0];
        
        /* Quantizer for low band */
        el = xl - wd1;
        
        /* Saturate to +/-32767 (16 bits) */
        wd1 = saturate(el);
        
        /* Then right shift by 1 to fit into 15 bits */
        wd1 >>= 1;
        
        s->dq[1] = quantize(wd1, s->yl, qm4, 9);
        
        wd1 = (s->dq[1] << 1) + s->a[0];
        s->sr[0] = saturate(wd1);
        
        /* Update low band predictor coefficients */
        s->a[0] = s->a[0] + ((s->dq[1] * 9) >> 5);
        
        /* Store the combined codeword */
        if (s->rate == 48000)
            g722_data[k] = (uint8_t) (s->dq[0] << 2) | (s->dq[1] & 0x3);
        else
            g722_data[k] = (uint8_t) (s->dq[0] << 6) | (s->dq[1] << 2);
    }
    
    return len;
} 