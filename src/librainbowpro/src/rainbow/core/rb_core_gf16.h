#ifndef rb__GF16_H_
#define rb__GF16_H_

#include <stdint.h>
#include "rb_core_config.h"

static inline uint8_t rb_gf4_mul_2(uint8_t a) {
    uint8_t r = a << 1;
    r ^= (a >> 1) * 7;
    return r;
}

static inline uint8_t rb_gf4_mul_3(uint8_t a) {
    uint8_t msk = (a - 2) >> 1;
    return (msk & (a * 3)) | ((~msk) & (a - 1));
}

static inline uint8_t rb_gf4_mul(uint8_t a, uint8_t b) {
    uint8_t r = a * (b & 1);
    return r ^ (rb_gf4_mul_2(a) * (b >> 1));
}

static inline uint8_t rb_gf4_squ(uint8_t a) {
    return a ^ (a >> 1);
}

static inline uint8_t rb_gf4_inv(uint8_t a) {
    return a ^ (a >> 1);
}

///////

static inline uint32_t rb_gf4v_mul_2_u32(uint32_t a) {
    uint32_t bit0 = a & 0x55555555;
    uint32_t bit1 = a & 0xaaaaaaaa;
    return (bit0 << 1) ^ bit1 ^ (bit1 >> 1);
}

static inline uint32_t rb_gf4v_mul_3_u32(uint32_t a) {
    uint32_t bit0 = a & 0x55555555;
    uint32_t bit1 = a & 0xaaaaaaaa;
    return (bit0 << 1) ^ bit0 ^ (bit1 >> 1);
}

static inline uint32_t rb_gf4v_mul_u32(uint32_t a, uint8_t b) {
    uint32_t bit0_b = ((uint32_t) 0) - ((uint32_t)(b & 1));
    uint32_t bit1_b = ((uint32_t) 0) - ((uint32_t)((b >> 1) & 1));
    return (a & bit0_b) ^ (bit1_b & rb_gf4v_mul_2_u32(a));
}

static inline uint32_t rb__gf4v_mul_u32_u32(uint32_t a0, uint32_t a1, uint32_t b0, uint32_t b1) {
    uint32_t c0 = a0 & b0;
    uint32_t c2 = a1 & b1;
    uint32_t c1_ = (a0 ^ a1) & (b0 ^ b1);
    return ((c1_ ^ c0) << 1) ^ c0 ^ c2;
}

static inline uint32_t rb_gf4v_mul_u32_u32(uint32_t a, uint32_t b) {
    uint32_t a0 = a & 0x55555555;
    uint32_t a1 = (a >> 1) & 0x55555555;
    uint32_t b0 = b & 0x55555555;
    uint32_t b1 = (b >> 1) & 0x55555555;

    return rb__gf4v_mul_u32_u32(a0, a1, b0, b1);
}

static inline uint32_t rb_gf4v_squ_u32(uint32_t a) {
    uint32_t bit1 = a & 0xaaaaaaaa;
    return a ^ (bit1 >> 1);
}

//////////////////////////////////////////////////////////////////////////////////

static inline uint8_t rb_gf16_is_nonzero(uint8_t a) {
    unsigned int a4 = a & 0xf;
    unsigned int r = ((unsigned int) 0) - a4;
    r >>= 4;
    return r & 1;
}


static inline uint8_t rb_gf16_mul(uint8_t a, uint8_t b) {
    uint8_t a0 = a & 3;
    uint8_t a1 = (a >> 2);
    uint8_t b0 = b & 3;
    uint8_t b1 = (b >> 2);
    uint8_t a0b0 = rb_gf4_mul(a0, b0);
    uint8_t a1b1 = rb_gf4_mul(a1, b1);
    uint8_t a0b1_a1b0 = rb_gf4_mul(a0 ^ a1, b0 ^ b1) ^ a0b0 ^ a1b1;
    uint8_t a1b1_x2 = rb_gf4_mul_2(a1b1);
    return ((a0b1_a1b0 ^ a1b1) << 2) ^ a0b0 ^ a1b1_x2;
}

static inline uint8_t rb_gf16_squ(uint8_t a) {
    uint8_t a0 = a & 3;
    uint8_t a1 = (a >> 2);
    a1 = rb_gf4_squ(a1);
    uint8_t a1squ_x2 = rb_gf4_mul_2(a1);
    return (a1 << 2) ^ a1squ_x2 ^ rb_gf4_squ(a0);
}

static inline uint8_t rb_gf16_inv(uint8_t a) {
    uint8_t a2 = rb_gf16_squ(a);
    uint8_t a4 = rb_gf16_squ(a2);
    uint8_t a8 = rb_gf16_squ(a4);
    uint8_t a6 = rb_gf16_mul(a4, a2);
    return rb_gf16_mul(a8, a6);
}

static inline uint8_t rb_gf16_mul_4(uint8_t a) {
    return (((a << 2) ^ a) & (8 + 4)) ^ rb_gf4_mul_2(a >> 2);
}

static inline uint8_t rb_gf16_mul_8(uint8_t a) {
    uint8_t a0 = a & 3;
    uint8_t a1 = a >> 2;
    return (rb_gf4_mul_2(a0 ^ a1) << 2) | rb_gf4_mul_3(a1);
}

////////////



static inline uint32_t rb_gf16v_mul_u32(uint32_t a, uint8_t b) {
    uint32_t axb0 = rb_gf4v_mul_u32(a, b);
    uint32_t axb1 = rb_gf4v_mul_u32(a, b >> 2);
    uint32_t a0b1 = (axb1 << 2) & 0xcccccccc;
    uint32_t a1b1 = axb1 & 0xcccccccc;
    uint32_t a1b1_2 = a1b1 >> 2;

    return axb0 ^ a0b1 ^ a1b1 ^ rb_gf4v_mul_2_u32(a1b1_2);
}

static inline uint32_t rb__gf16v_mul_u32_u32(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t b0, uint32_t b1, uint32_t b2, uint32_t b3) {
    uint32_t c0 = rb__gf4v_mul_u32_u32(a0, a1, b0, b1);
    uint32_t c1_ = rb__gf4v_mul_u32_u32(a0 ^ a2, a1 ^ a3, b0 ^ b2, b1 ^ b3);

    uint32_t c2_0 = a2 & b2;
    uint32_t c2_2 = a3 & b3;
    uint32_t c2_1_ = (a2 ^ a3) & (b2 ^ b3);
    uint32_t c2_r0 = c2_0 ^ c2_2;
    uint32_t c2_r1 = c2_0 ^ c2_1_;

    return ((c1_ ^ c0) << 2) ^ c0 ^ (c2_r0 << 1) ^ c2_r1 ^ (c2_r1 << 1);
}

static inline uint32_t rb_gf16v_mul_u32_u32(uint32_t a, uint32_t b) {
    uint32_t a0 = a & 0x11111111;
    uint32_t a1 = (a >> 1) & 0x11111111;
    uint32_t a2 = (a >> 2) & 0x11111111;
    uint32_t a3 = (a >> 3) & 0x11111111;
    uint32_t b0 = b & 0x11111111;
    uint32_t b1 = (b >> 1) & 0x11111111;
    uint32_t b2 = (b >> 2) & 0x11111111;
    uint32_t b3 = (b >> 3) & 0x11111111;

    return rb__gf16v_mul_u32_u32(a0, a1, a2, a3, b0, b1, b2, b3);
}

static inline uint8_t rb_gf256v_reduce_u32(uint32_t a) {

    uint16_t *aa = (uint16_t *) (&a);
    uint16_t r = aa[0] ^ aa[1];
    uint8_t *rr = (uint8_t *) (&r);
    return rr[0] ^ rr[1];

}

static inline uint8_t rb_gf16v_reduce_u32(uint32_t a) {
    uint8_t r256 = rb_gf256v_reduce_u32(a);
    return (r256 & 0xf) ^ (r256 >> 4);
}

static inline uint32_t rb_gf16v_squ_u32(uint32_t a) {
    uint32_t a2 = rb_gf4v_squ_u32(a);

    return a2 ^ rb_gf4v_mul_2_u32((a2 >> 2) & 0x33333333);
}

static inline uint32_t rb_gf16v_mul_4_u32(uint32_t a) {
    uint32_t a1 = a & 0xcccccccc;
    uint32_t a0 = (a << 2) & 0xcccccccc;
    return a0 ^ a1 ^ rb_gf4v_mul_2_u32(a1 >> 2);
}

static inline uint32_t rb_gf16v_mul_8_u32(uint32_t a) {
    uint32_t a1 = a & 0xcccccccc;
    uint32_t a0 = (a << 2) & 0xcccccccc;
    return rb_gf4v_mul_2_u32(a0 ^ a1) | rb_gf4v_mul_3_u32(a1 >> 2);
}

////////////

static inline uint8_t rb_gf256_is_nonzero(uint8_t a) {
    unsigned int a8 = a;
    unsigned int r = ((unsigned int) 0) - a8;
    r >>= 8;
    return r & 1;
}


static inline uint8_t rb_gf256_mul(uint8_t a, uint8_t b) {
    uint8_t a0 = a & 15;
    uint8_t a1 = (a >> 4);
    uint8_t b0 = b & 15;
    uint8_t b1 = (b >> 4);
    uint8_t a0b0 = rb_gf16_mul(a0, b0);
    uint8_t a1b1 = rb_gf16_mul(a1, b1);
    uint8_t a0b1_a1b0 = rb_gf16_mul(a0 ^ a1, b0 ^ b1) ^ a0b0 ^ a1b1;
    uint8_t a1b1_x8 = rb_gf16_mul_8(a1b1);
    return ((a0b1_a1b0 ^ a1b1) << 4) ^ a0b0 ^ a1b1_x8;
}

static inline uint8_t rb_gf256_mul_gf16(uint8_t a, uint8_t gf16_b) {
    uint8_t a0 = a & 15;
    uint8_t a1 = (a >> 4);
    uint8_t b0 = gf16_b & 15;
    uint8_t a0b0 = rb_gf16_mul(a0, b0);
    uint8_t a1b0 = rb_gf16_mul(a1, b0);
    return a0b0 ^ (a1b0 << 4);
}

static inline uint8_t rb_gf256_squ(uint8_t a) {
    uint8_t a0 = a & 15;
    uint8_t a1 = (a >> 4);
    a1 = rb_gf16_squ(a1);
    uint8_t a1squ_x8 = rb_gf16_mul_8(a1);
    return (a1 << 4) ^ a1squ_x8 ^ rb_gf16_squ(a0);
}

static inline uint8_t rb_gf256_inv(uint8_t a) {

    uint8_t a2 = rb_gf256_squ(a);
    uint8_t a4 = rb_gf256_squ(a2);
    uint8_t a8 = rb_gf256_squ(a4);
    uint8_t a4_2 = rb_gf256_mul(a4, a2);
    uint8_t a8_4_2 = rb_gf256_mul(a4_2, a8);
    uint8_t a64_ = rb_gf256_squ(a8_4_2);
    a64_ = rb_gf256_squ(a64_);
    a64_ = rb_gf256_squ(a64_);
    uint8_t a64_2 = rb_gf256_mul(a64_, a8_4_2);
    uint8_t a128_ = rb_gf256_squ(a64_2);
    return rb_gf256_mul(a2, a128_);
}



static inline uint32_t rb_gf256v_mul_u32(uint32_t a, uint8_t b) {
    uint32_t axb0 = rb_gf16v_mul_u32(a, b);
    uint32_t axb1 = rb_gf16v_mul_u32(a, b >> 4);
    uint32_t a0b1 = (axb1 << 4) & 0xf0f0f0f0;
    uint32_t a1b1 = axb1 & 0xf0f0f0f0;
    uint32_t a1b1_4 = a1b1 >> 4;

    return axb0 ^ a0b1 ^ a1b1 ^ rb_gf16v_mul_8_u32(a1b1_4);
}

static inline uint32_t rb_gf256v_squ_u32(uint32_t a) {
    uint32_t a2 = rb_gf16v_squ_u32(a);
    uint32_t ar = (a2 >> 4) & 0x0f0f0f0f;

    return a2 ^ rb_gf16v_mul_8_u32(ar);
}

static inline uint32_t rb_gf256v_mul_gf16_u32(uint32_t a, uint8_t gf16_b) {
    return rb_gf16v_mul_u32(a, gf16_b);
}



static inline uint32_t rb_gf256v_mul_0x10_u32(uint32_t a) {
    uint32_t a0 = a&0x0f0f0f0f;
    uint32_t a1 = a&0xf0f0f0f0;
    uint32_t a1x = rb_gf16v_mul_8_u32(a1>>4);
    return (a0<<4)^a1^a1x;
}

static inline uint32_t rb_gf256v_mul_0x20_u32(uint32_t a) {
    uint32_t a0 = rb_gf4v_mul_2_u32(a&0x0f0f0f0f);
    uint32_t a1 = rb_gf4v_mul_2_u32(a&0xf0f0f0f0);
    uint32_t a1x = rb_gf16v_mul_8_u32(a1>>4);
    return (a0<<4)^a1^a1x;
}

static inline uint32_t rb_gf256v_mul_0x40_u32(uint32_t a) {
    uint32_t a0 = rb_gf16v_mul_4_u32(a&0x0f0f0f0f);
    uint32_t a1 = rb_gf16v_mul_4_u32(a&0xf0f0f0f0);
    uint32_t a1x = rb_gf16v_mul_8_u32(a1>>4);
    return (a0<<4)^a1^a1x;
}

static inline uint32_t rb_gf256v_mul_0x80_u32(uint32_t a) {
    uint32_t a0 = rb_gf16v_mul_8_u32(a&0x0f0f0f0f);
    uint32_t a1 = rb_gf16v_mul_8_u32(a&0xf0f0f0f0);
    uint32_t a1x = rb_gf16v_mul_8_u32(a1>>4);
    return (a0<<4)^a1^a1x;
}

#endif // rb__GF16_H_

