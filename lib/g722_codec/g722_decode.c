/**
 * @file g722_decode.c
 * @brief G.722 decoder implementation
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

struct g722_decode_state_s {
    int yl;
    int yu;
    int dms;
    int dml;
    int ap[2];
    int a[2][2];
    int b[2][6];
    int pk[2][3];
    int dq[2][6];
    int sr[2][2];
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

static int16_t reconstruct(int sign, int dqln, int y) {
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

g722_decode_state_t *g722_decoder_init(int rate, int options) {
    g722_decode_state_t *s;
    
    if ((s = (g722_decode_state_t *) malloc(sizeof(*s))) == NULL)
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

void g722_decoder_release(g722_decode_state_t *s) {
    free(s);
}

int g722_decode(g722_decode_state_t *s, int16_t pcm_data[], const uint8_t g722_data[], int len) {
    int dlowt;
    int rlow;
    int ihigh;
    int ilow;
    int dhigh;
    int rhigh;
    int xout1;
    int xout2;
    int wd1;
    int wd2;
    int wd3;
    int i;
    int code;
    int outlen;
    
    outlen = 0;
    
    for (i = 0; i < len; i++) {
        code = g722_data[i];
        
        switch (s->rate) {
            default:
            case 64000:
                /* Get the upper 6 bits of the code */
                ihigh = (code >> 2) & 0x3F;
                /* Get the lower 2 bits of the code */
                ilow = code & 0x03;
                /* Only 14 bits of the code are effective for 64 kb/s */
                ilow <<= 2;
                break;
            case 56000:
                /* Get the upper 6 bits of the code */
                ihigh = (code >> 1) & 0x3F;
                /* Get the lower 1 bit of the code */
                ilow = code & 0x01;
                /* Only 13 bits of the code are effective for 56 kb/s */
                ilow <<= 2;
                break;
            case 48000:
                /* Get the upper 6 bits of the code */
                ihigh = code & 0x3F;
                /* Get the lower 0 bits of the code */
                ilow = 0;
                /* Only 12 bits of the code are effective for 48 kb/s */
                break;
        }
        
        /* Block 5L, Low band ADPCM reconstruction */
        
        /* Block 5L, low band decoder, for QI = 2 */
        wd1 = saturate(s->a[1][0]);
        wd2 = (s->a[1][1] * 32512) >> 15;
        wd3 = ((ilow << 14) + wd1 - wd2) >> 13;
        
        if (wd3 < 0)
            ilow = ((-wd3) >> 10) & 1;
        else
            ilow = 0;
        
        /* Block 5L, RECONS */
        /* Compute quantized difference signal */
        wd2 = qm4[ilow << 2];
        
        /* Scale and update low band quantizer adaptation */
        wd2 = (s->yu * wd2) >> 15;
        
        /* Compute quantized difference signal */
        if (wd2 > 16383)
            wd2 = 16383;
        else if (wd2 < -16384)
            wd2 = -16384;
        
        wd2 <<= 1;
        
        dlowt = wd2;
        
        /* Reconstruct the signal */
        rlow = saturate(s->yl >> 15) + dlowt;
        
        /* Block 5L, PARREC */
        s->pk[1][0] = s->pk[1][1];
        s->pk[1][1] = rlow;
        
        /* Block 5L, UPZERO */
        wd1 = dlowt >> 31;
        wd1 = (wd1 ^ s->dq[1][0]) + (wd1 & 1);
        s->dq[1][0] = dlowt;
        wd2 = (s->pk[1][0] * wd1) >> 8;
        if (wd2 > 32767)
            wd2 = 32767;
        wd3 = (s->pk[1][1] * wd1) >> 8;
        if (wd3 > 32767)
            wd3 = 32767;
        s->pk[1][0] = saturate(s->pk[1][0] - wd2);
        s->pk[1][1] = saturate(s->pk[1][1] - wd3);
        
        /* Block 5L, UPPOL2 */
        wd1 = saturate(s->yl >> 15);
        if (wd1 == 0)
            wd2 = 0;
        else
            wd2 = saturate(s->yu * wd1) >> 15;
        if (wd2 == 0)
            wd3 = 0;
        else
            wd3 = (s->yu * wd2) >> 15;
        s->yu = wd3;
        
        /* Block 6H, High band ADPCM reconstruction */
        
        /* Block 6H, high band decoder, for QI = 2 */
        wd1 = saturate(s->a[0][0]);
        wd2 = (s->a[0][1] * 32512) >> 15;
        wd3 = ((ihigh << 14) + wd1 - wd2) >> 13;
        
        if (wd3 < 0)
            ihigh = ((-wd3) >> 10) & 0x7;
        else
            ihigh = 0;
        
        /* Block 6H, RECONS */
        /* Compute quantized difference signal */
        wd2 = qm4[ihigh << 1];
        
        /* Scale and update high band quantizer adaptation */
        wd2 = (s->yu * wd2) >> 15;
        
        /* Compute quantized difference signal */
        if (wd2 > 16383)
            wd2 = 16383;
        else if (wd2 < -16384)
            wd2 = -16384;
        
        wd2 <<= 1;
        
        dhigh = wd2;
        
        /* Reconstruct the signal */
        rhigh = saturate(s->yl >> 15) + dhigh;
        
        /* Block 6H, PARREC */
        s->pk[0][0] = s->pk[0][1];
        s->pk[0][1] = rhigh;
        
        /* Block 6H, UPZERO */
        wd1 = dhigh >> 31;
        wd1 = (wd1 ^ s->dq[0][0]) + (wd1 & 1);
        s->dq[0][0] = dhigh;
        wd2 = (s->pk[0][0] * wd1) >> 8;
        if (wd2 > 32767)
            wd2 = 32767;
        wd3 = (s->pk[0][1] * wd1) >> 8;
        if (wd3 > 32767)
            wd3 = 32767;
        s->pk[0][0] = saturate(s->pk[0][0] - wd2);
        s->pk[0][1] = saturate(s->pk[0][1] - wd3);
        
        /* Block 6H, UPPOL2 */
        wd1 = saturate(s->yl >> 15);
        if (wd1 == 0)
            wd2 = 0;
        else
            wd2 = saturate(s->yu * wd1) >> 15;
        if (wd2 == 0)
            wd3 = 0;
        else
            wd3 = (s->yu * wd2) >> 15;
        s->yu = wd3;
        
        /* Recombine the bands */
        xout1 = saturate(rlow + rhigh);
        xout2 = saturate(rlow - rhigh);
        
        /* Save the current output sample */
        pcm_data[outlen++] = (int16_t) ((xout1 << 8) | (xout2 & 0xFF));
    }
    
    return outlen;
} 