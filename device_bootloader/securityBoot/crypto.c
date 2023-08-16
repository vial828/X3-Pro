#include "crypto.h"

#include <string.h>

#include "stdio.h"
#define NUM_cryalg_DIGITS (cryalg_BYTES/8)
#define MAX_TRIES 16
typedef unsigned int uint;

typedef struct
{
    uint64_t m_low;
    uint64_t m_high;
} uint128_t;

typedef struct cryalgPoint
{
    uint64_t x[NUM_cryalg_DIGITS];
    uint64_t y[NUM_cryalg_DIGITS];
} cryalgPoint;

#define CONCAT1(a, b) a##b
#define CONCAT(a, b) CONCAT1(a, b)

#define Curve_P_16 {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFDFFFFFFFF}
#define Curve_P_24 {0xFFFFFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFEull, 0xFFFFFFFFFFFFFFFFull}
#define Curve_P_32 {0xFFFFFFFFFFFFFFFFull, 0x00000000FFFFFFFFull, 0x0000000000000000ull, 0xFFFFFFFF00000001ull}
#define Curve_P_48 {0x00000000FFFFFFFF, 0xFFFFFFFF00000000, 0xFFFFFFFFFFFFFFFE, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF}

#define Curve_B_16 {0xD824993C2CEE5ED3, 0xE87579C11079F43D}
#define Curve_B_24 {0xFEB8DEcryalg146B9B1ull, 0x0FA7E9AB72243049ull, 0x64210519E59C80E7ull}
#define Curve_B_32 {0x3BCE3C3E27D2604Bull, 0x651D06B0CC53B0F6ull, 0xB3EBBD55769886BCull, 0x5AC635D8AA3A93E7ull}
#define Curve_B_48 {0x2A85C8EDD3EC2AEF, 0xC656398D8A2ED19D, 0x0314088F5013875A, 0x181D9C6EFE814112, 0x988E056BE3F82D19, 0xB3312FA7E23EE7E4}

#define Curve_G_16 { \
    {0x0C28607CA52C5B86, 0x161FF7528B899B2D}, \
    {0xC02DA292DDED7A83, 0xCF5AC8395BAFEB13}}

#define Curve_G_24 { \
    {0xF4FF0AFD82FF1012ull, 0x7CBF20EB43A18800ull, 0x188DA80EB03090F6ull}, \
    {0x73F977A11E794811ull, 0x631011ED6B24CDD5ull, 0x07192B95FFC8DA78ull}}
    
#define Curve_G_32 { \
    {0xF4A13945D898C296ull, 0x77037D812DEB33A0ull, 0xF8BCE6E563A440F2ull, 0x6B17D1F2E12C4247ull}, \
    {0xCBB6406837BF51F5ull, 0x2BCE33576B315ECEull, 0x8EE7EB4A7C0F9E16ull, 0x4FE342E2FE1A7F9Bull}}

#define Curve_G_48 { \
    {0x3A545E3872760AB7, 0x5502F25DBF55296C, 0x59F741E082542A38, 0x6E1D3B628BA79B98, 0x8EB1C71EF320AD74, 0xAA87CA22BE8B0537}, \
    {0x7A431D7C90EA0E5F, 0x0A60B1CE1D7E819D, 0xE9DA3113B5F0B8C0, 0xF8F41DBD289A147C, 0x5D9E98BF9292DC29, 0x3617DE4A96262C6F}}

#define Curve_N_16 {0x75A30D1B9038A115, 0xFFFFFFFE00000000}
#define Curve_N_24 {0x146BC9B1B4D22831ull, 0xFFFFFFFF99DEF836ull, 0xFFFFFFFFFFFFFFFFull}
#define Curve_N_32 {0xF3B9CAC2FC632551ull, 0xBCE6FAADA7179E84ull, 0xFFFFFFFFFFFFFFFFull, 0xFFFFFFFF00000000ull}
#define Curve_N_48 {0xECEC196ACCC52973, 0x581A0DB248B0A77A, 0xC7634D81F4372DDF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF}

static uint64_t curve_p[NUM_cryalg_DIGITS] = CONCAT(Curve_P_, cryalg_CURVE);
static uint64_t curve_b[NUM_cryalg_DIGITS] = CONCAT(Curve_B_, cryalg_CURVE);
static cryalgPoint curve_G = CONCAT(Curve_G_, cryalg_CURVE);
static uint64_t curve_n[NUM_cryalg_DIGITS] = CONCAT(Curve_N_, cryalg_CURVE);



//#define O_CLOEXEC 0


static void vli_clear(uint64_t *p_vli)
{
    uint i;
    for(i=0; i<NUM_cryalg_DIGITS; ++i){
        p_vli[i] = 0;
    }
}

static int vli_isZero(uint64_t *p_vli)
{
    uint i;
    for(i = 0; i < NUM_cryalg_DIGITS; ++i){
        if(p_vli[i]){
            return 0;
        }
    }
    return 1;
}

static uint64_t vli_testBit(uint64_t *p_vli, uint p_bit)
{
    return (p_vli[p_bit/64] & ((uint64_t)1 << (p_bit % 64)));
}

static uint vli_numDigits(uint64_t *p_vli)
{
    int i;
    for(i = NUM_cryalg_DIGITS - 1; i >= 0 && p_vli[i] == 0; --i){}
    return (i + 1);
}
static uint vli_numBits(uint64_t *p_vli)
{
    uint i;
    uint64_t l_digit;
    
    uint l_numDigits = vli_numDigits(p_vli);
    if(l_numDigits == 0){
        return 0;
    }

    l_digit = p_vli[l_numDigits - 1];
    for(i=0; l_digit; ++i){
        l_digit >>= 1;
    }
    
    return ((l_numDigits - 1) * 64 + i);
}

static void vli_set(uint64_t *p_dest, uint64_t *p_src)
{
    uint i;
    for(i=0; i<NUM_cryalg_DIGITS; ++i){
        p_dest[i] = p_src[i];
    }
}

static int vli_cmp(uint64_t *p_left, uint64_t *p_right)
{
    int i;
    for(i = NUM_cryalg_DIGITS-1; i >= 0; --i){
        if(p_left[i] > p_right[i]){
            return 1;
        }else if(p_left[i] < p_right[i]){
            return -1;
        }
    }
    return 0;
}

static uint64_t vli_lshift(uint64_t *p_result, uint64_t *p_in, uint p_shift)
{
    uint64_t l_carry = 0;
    uint i;
    for(i = 0; i < NUM_cryalg_DIGITS; ++i){
        uint64_t l_temp = p_in[i];
        p_result[i] = (l_temp << p_shift) | l_carry;
        l_carry = l_temp >> (64 - p_shift);
    }
    
    return l_carry;
}

static void vli_rshift1(uint64_t *p_vli)
{
    uint64_t *l_end = p_vli;
    uint64_t l_carry = 0;
    
    p_vli += NUM_cryalg_DIGITS;
    while(p_vli-- > l_end){
        uint64_t l_temp = *p_vli;
        *p_vli = (l_temp >> 1) | l_carry;
        l_carry = l_temp << 63;
    }
}

static uint64_t vli_add(uint64_t *p_result, uint64_t *p_left, uint64_t *p_right)
{
    uint64_t l_carry = 0;
    uint i;
    for(i=0; i<NUM_cryalg_DIGITS; ++i){
        uint64_t l_sum = p_left[i] + p_right[i] + l_carry;
        if(l_sum != p_left[i]){
            l_carry = (l_sum < p_left[i]);
        }
        p_result[i] = l_sum;
    }
    return l_carry;
}

static uint64_t vli_sub(uint64_t *p_result, uint64_t *p_left, uint64_t *p_right)
{
    uint64_t l_borrow = 0;
    uint i;
    for(i=0; i<NUM_cryalg_DIGITS; ++i){
        uint64_t l_diff = p_left[i] - p_right[i] - l_borrow;
        if(l_diff != p_left[i]){
            l_borrow = (l_diff > p_left[i]);
        }
        p_result[i] = l_diff;
    }
    return l_borrow;
}


static uint128_t mul_64_64(uint64_t p_left, uint64_t p_right)
{
    uint128_t l_result;
    
    uint64_t a0 = p_left & 0xffffffffull;
    uint64_t a1 = p_left >> 32;
    uint64_t b0 = p_right & 0xffffffffull;
    uint64_t b1 = p_right >> 32;
    
    uint64_t m0 = a0 * b0;
    uint64_t m1 = a0 * b1;
    uint64_t m2 = a1 * b0;
    uint64_t m3 = a1 * b1;
    
    m2 += (m0 >> 32);
    m2 += m1;
    if(m2 < m1){ 
        m3 += 0x100000000ull;
    }
    
    l_result.m_low = (m0 & 0xffffffffull) | (m2 << 32);
    l_result.m_high = m3 + (m2 >> 32);
    
    return l_result;
}

static uint128_t add_128_128(uint128_t a, uint128_t b)
{
    uint128_t l_result;
    l_result.m_low = a.m_low + b.m_low;
    l_result.m_high = a.m_high + b.m_high + (l_result.m_low < a.m_low);
    return l_result;
}

static void vli_mult(uint64_t *p_result, uint64_t *p_left, uint64_t *p_right)
{
    uint128_t r01 = {0, 0};
    uint64_t r2 = 0;
    
    uint i, k;
    
    for(k=0; k < NUM_cryalg_DIGITS*2 - 1; ++k){
        uint l_min = (k < NUM_cryalg_DIGITS ? 0 : (k + 1) - NUM_cryalg_DIGITS);
        for(i=l_min; i<=k && i<NUM_cryalg_DIGITS; ++i)
        {
            uint128_t l_product = mul_64_64(p_left[i], p_right[k-i]);
            r01 = add_128_128(r01, l_product);
            r2 += (r01.m_high < l_product.m_high);
        }
        p_result[k] = r01.m_low;
        r01.m_low = r01.m_high;
        r01.m_high = r2;
        r2 = 0;
    }
    
    p_result[NUM_cryalg_DIGITS*2 - 1] = r01.m_low;
}

static void vli_square(uint64_t *p_result, uint64_t *p_left)
{
    uint128_t r01 = {0, 0};
    uint64_t r2 = 0;
    
    uint i, k;
    for(k=0; k < NUM_cryalg_DIGITS*2 - 1; ++k){
        uint l_min = (k < NUM_cryalg_DIGITS ? 0 : (k + 1) - NUM_cryalg_DIGITS);
        for(i=l_min; i<=k && i<=k-i; ++i){
            uint128_t l_product = mul_64_64(p_left[i], p_left[k-i]);
            if(i < k-i){
                r2 += l_product.m_high >> 63;
                l_product.m_high = (l_product.m_high << 1) | (l_product.m_low >> 63);
                l_product.m_low <<= 1;
            }
            r01 = add_128_128(r01, l_product);
            r2 += (r01.m_high < l_product.m_high);
        }
        p_result[k] = r01.m_low;
        r01.m_low = r01.m_high;
        r01.m_high = r2;
        r2 = 0;
    }
    
    p_result[NUM_cryalg_DIGITS*2 - 1] = r01.m_low;
}

static void vli_modAdd(uint64_t *p_result, uint64_t *p_left, uint64_t *p_right, uint64_t *p_mod)
{
    uint64_t l_carry = vli_add(p_result, p_left, p_right);
    if(l_carry || vli_cmp(p_result, p_mod) >= 0){ 
        vli_sub(p_result, p_result, p_mod);
    }
}

static void vli_modSub(uint64_t *p_result, uint64_t *p_left, uint64_t *p_right, uint64_t *p_mod)
{
    uint64_t l_borrow = vli_sub(p_result, p_left, p_right);
    if(l_borrow){ 
        vli_add(p_result, p_result, p_mod);
    }
}


static void vli_mmod_fast(uint64_t *p_result, uint64_t *p_product)
{
    uint64_t l_tmp[NUM_cryalg_DIGITS];
    int l_carry;
    
    vli_set(p_result, p_product);
    
    l_tmp[0] = p_product[2];
    l_tmp[1] = (p_product[3] & 0x1FFFFFFFFull) | (p_product[2] << 33);
    l_carry = vli_add(p_result, p_result, l_tmp);
    
    l_tmp[0] = (p_product[2] >> 31) | (p_product[3] << 33);
    l_tmp[1] = (p_product[3] >> 31) | ((p_product[2] & 0xFFFFFFFF80000000ull) << 2);
    l_carry += vli_add(p_result, p_result, l_tmp);
    
    l_tmp[0] = (p_product[2] >> 62) | (p_product[3] << 2);
    l_tmp[1] = (p_product[3] >> 62) | ((p_product[2] & 0xC000000000000000ull) >> 29) | (p_product[3] << 35);
    l_carry += vli_add(p_result, p_result, l_tmp);
    
    l_tmp[0] = (p_product[3] >> 29);
    l_tmp[1] = ((p_product[3] & 0xFFFFFFFFE0000000ull) << 4);
    l_carry += vli_add(p_result, p_result, l_tmp);
    
    l_tmp[0] = (p_product[3] >> 60);
    l_tmp[1] = (p_product[3] & 0xFFFFFFFE00000000ull);
    l_carry += vli_add(p_result, p_result, l_tmp);
    
    l_tmp[0] = 0;
    l_tmp[1] = ((p_product[3] & 0xF000000000000000ull) >> 27);
    l_carry += vli_add(p_result, p_result, l_tmp);
    
    while(l_carry || vli_cmp(curve_p, p_result) != 1){
        l_carry -= vli_sub(p_result, p_result, curve_p);
    }
}


static void vli_modMult_fast(uint64_t *p_result, uint64_t *p_left, uint64_t *p_right)
{
    uint64_t l_product[2 * NUM_cryalg_DIGITS];
    vli_mult(l_product, p_left, p_right);
    vli_mmod_fast(p_result, l_product);
}

static void vli_modSquare_fast(uint64_t *p_result, uint64_t *p_left)
{
    uint64_t l_product[2 * NUM_cryalg_DIGITS];
    vli_square(l_product, p_left);
    vli_mmod_fast(p_result, l_product);
}

#define EVEN(vli) (!(vli[0] & 1))
static void vli_modInv(uint64_t *p_result, uint64_t *p_input, uint64_t *p_mod)
{
    uint64_t a[NUM_cryalg_DIGITS], b[NUM_cryalg_DIGITS], u[NUM_cryalg_DIGITS], v[NUM_cryalg_DIGITS];
    uint64_t l_carry;
    int l_cmpResult;
    
    if(vli_isZero(p_input)){
        vli_clear(p_result);
        return;
    }

    vli_set(a, p_input);
    vli_set(b, p_mod);
    vli_clear(u);
    u[0] = 1;
    vli_clear(v);
    
    while((l_cmpResult = vli_cmp(a, b)) != 0){
        l_carry = 0;
        if(EVEN(a)){
            vli_rshift1(a);
            if(!EVEN(u)){
                l_carry = vli_add(u, u, p_mod);
            }
            vli_rshift1(u);
            if(l_carry){
                u[NUM_cryalg_DIGITS-1] |= 0x8000000000000000ull;
            }
        }else if(EVEN(b)){
            vli_rshift1(b);
            if(!EVEN(v)){
                l_carry = vli_add(v, v, p_mod);
            }
            vli_rshift1(v);
            if(l_carry){
                v[NUM_cryalg_DIGITS-1] |= 0x8000000000000000ull;
            }
        }
        else if(l_cmpResult > 0){
            vli_sub(a, a, b);
            vli_rshift1(a);
            if(vli_cmp(u, v) < 0){
                vli_add(u, u, p_mod);
            }
            vli_sub(u, u, v);
            if(!EVEN(u)){
                l_carry = vli_add(u, u, p_mod);
            }
            vli_rshift1(u);
            if(l_carry){
                u[NUM_cryalg_DIGITS-1] |= 0x8000000000000000ull;
            }
        }
        else{
            vli_sub(b, b, a);
            vli_rshift1(b);
            if(vli_cmp(v, u) < 0){
                vli_add(v, v, p_mod);
            }
            vli_sub(v, v, u);
            if(!EVEN(v)){
                l_carry = vli_add(v, v, p_mod);
            }
            vli_rshift1(v);
            if(l_carry){
                v[NUM_cryalg_DIGITS-1] |= 0x8000000000000000ull;
            }
        }
    }
    
    vli_set(p_result, u);
}


static void cryalgPoint_double_jacobian(uint64_t *X1, uint64_t *Y1, uint64_t *Z1)
{
    uint64_t t4[NUM_cryalg_DIGITS];
    uint64_t t5[NUM_cryalg_DIGITS];
    
    if(vli_isZero(Z1)){
        return;
    }
    
    vli_modSquare_fast(t4, Y1);   
    vli_modMult_fast(t5, X1, t4);
    vli_modSquare_fast(t4, t4);  
    vli_modMult_fast(Y1, Y1, Z1);
    vli_modSquare_fast(Z1, Z1);   
    
    vli_modAdd(X1, X1, Z1, curve_p); 
    vli_modAdd(Z1, Z1, Z1, curve_p); 
    vli_modSub(Z1, X1, Z1, curve_p); 
    vli_modMult_fast(X1, X1, Z1);    
    
    vli_modAdd(Z1, X1, X1, curve_p); 
    vli_modAdd(X1, X1, Z1, curve_p); 
    if(vli_testBit(X1, 0)){
        uint64_t l_carry = vli_add(X1, X1, curve_p);
        vli_rshift1(X1);
        X1[NUM_cryalg_DIGITS-1] |= l_carry << 63;
    }else{
        vli_rshift1(X1);
    }
    
    vli_modSquare_fast(Z1, X1);      
    vli_modSub(Z1, Z1, t5, curve_p); 
    vli_modSub(Z1, Z1, t5, curve_p); 
    vli_modSub(t5, t5, Z1, curve_p); 
    vli_modMult_fast(X1, X1, t5);    
    vli_modSub(t4, X1, t4, curve_p); 
    
    vli_set(X1, Z1);
    vli_set(Z1, Y1);
    vli_set(Y1, t4);
}

static void apply_z(uint64_t *X1, uint64_t *Y1, uint64_t *Z)
{
    uint64_t t1[NUM_cryalg_DIGITS];

    vli_modSquare_fast(t1, Z);    
    vli_modMult_fast(X1, X1, t1); 
    vli_modMult_fast(t1, t1, Z);  
    vli_modMult_fast(Y1, Y1, t1); 
}


static void XYcZ_add(uint64_t *X1, uint64_t *Y1, uint64_t *X2, uint64_t *Y2)
{
    uint64_t t5[NUM_cryalg_DIGITS];
    
    vli_modSub(t5, X2, X1, curve_p); 
    vli_modSquare_fast(t5, t5);      
    vli_modMult_fast(X1, X1, t5);    
    vli_modMult_fast(X2, X2, t5);    
    vli_modSub(Y2, Y2, Y1, curve_p); 
    vli_modSquare_fast(t5, Y2);      
    
    vli_modSub(t5, t5, X1, curve_p); 
    vli_modSub(t5, t5, X2, curve_p); 
    vli_modSub(X2, X2, X1, curve_p); 
    vli_modMult_fast(Y1, Y1, X2);    
    vli_modSub(X2, X1, t5, curve_p); 
    vli_modMult_fast(Y2, Y2, X2);    
    vli_modSub(Y2, Y2, Y1, curve_p);     
    
    vli_set(X2, t5);
}



static void cryalg_bytes2native(uint64_t p_native[NUM_cryalg_DIGITS], const uint8_t p_bytes[cryalg_BYTES])
{
    unsigned i;
    for(i=0; i<NUM_cryalg_DIGITS; ++i)
    {
        const uint8_t *p_digit = p_bytes + 8 * (NUM_cryalg_DIGITS - 1 - i);
        p_native[i] = ((uint64_t)p_digit[0] << 56) | ((uint64_t)p_digit[1] << 48) | ((uint64_t)p_digit[2] << 40) | ((uint64_t)p_digit[3] << 32) |
            ((uint64_t)p_digit[4] << 24) | ((uint64_t)p_digit[5] << 16) | ((uint64_t)p_digit[6] << 8) | (uint64_t)p_digit[7];
    }
}


static void mod_sqrt(uint64_t a[NUM_cryalg_DIGITS])
{
    unsigned i;
    uint64_t p1[NUM_cryalg_DIGITS] = {1};
    uint64_t l_result[NUM_cryalg_DIGITS] = {1};
    vli_add(p1, curve_p, p1); 
    for(i = vli_numBits(p1) - 1; i > 1; --i)
    {
        vli_modSquare_fast(l_result, l_result);
        if(vli_testBit(p1, i))
        {
            vli_modMult_fast(l_result, l_result, a);
        }
    }
    vli_set(a, l_result);
}

static void cryalg_point_decompress(cryalgPoint *p_point, const uint8_t p_compressed[cryalg_BYTES+1])
{
    uint64_t _3[NUM_cryalg_DIGITS] = {3}; 
    cryalg_bytes2native(p_point->x, p_compressed+1);
    
    vli_modSquare_fast(p_point->y, p_point->x); 
    vli_modSub(p_point->y, p_point->y, _3, curve_p); 
    vli_modMult_fast(p_point->y, p_point->y, p_point->x); 
    vli_modAdd(p_point->y, p_point->y, curve_b, curve_p); 
    
    mod_sqrt(p_point->y);
    
    if((p_point->y[0] & 0x01) != (p_compressed[0] & 0x01))
    {
        vli_sub(p_point->y, curve_p, p_point->y);
    }
}



static void vli_modMult(uint64_t *p_result, uint64_t *p_left, uint64_t *p_right, uint64_t *p_mod)
{
    uint64_t l_product[2 * NUM_cryalg_DIGITS];
    uint64_t l_modMultiple[2 * NUM_cryalg_DIGITS];
    uint l_digitShift, l_bitShift;
    uint l_productBits;
    uint l_modBits = vli_numBits(p_mod);
    
    vli_mult(l_product, p_left, p_right);
    l_productBits = vli_numBits(l_product + NUM_cryalg_DIGITS);
    if(l_productBits)
    {
        l_productBits += NUM_cryalg_DIGITS * 64;
    }
    else
    {
        l_productBits = vli_numBits(l_product);
    }
    
    if(l_productBits < l_modBits)
    { 
        vli_set(p_result, l_product);
        return;
    }
    
    vli_clear(l_modMultiple);
    vli_clear(l_modMultiple + NUM_cryalg_DIGITS);
    l_digitShift = (l_productBits - l_modBits) / 64;
    l_bitShift = (l_productBits - l_modBits) % 64;
    if(l_bitShift)
    {
        l_modMultiple[l_digitShift + NUM_cryalg_DIGITS] = vli_lshift(l_modMultiple + l_digitShift, p_mod, l_bitShift);
    }
    else
    {
        vli_set(l_modMultiple + l_digitShift, p_mod);
    }

    vli_clear(p_result);
    p_result[0] = 1; 
    while(l_productBits > NUM_cryalg_DIGITS * 64 || vli_cmp(l_modMultiple, p_mod) >= 0)
    {
        int l_cmp = vli_cmp(l_modMultiple + NUM_cryalg_DIGITS, l_product + NUM_cryalg_DIGITS);
        if(l_cmp < 0 || (l_cmp == 0 && vli_cmp(l_modMultiple, l_product) <= 0))
        {
            if(vli_sub(l_product, l_product, l_modMultiple))
            { 
                vli_sub(l_product + NUM_cryalg_DIGITS, l_product + NUM_cryalg_DIGITS, p_result);
            }
            vli_sub(l_product + NUM_cryalg_DIGITS, l_product + NUM_cryalg_DIGITS, l_modMultiple + NUM_cryalg_DIGITS);
        }
        uint64_t l_carry = (l_modMultiple[NUM_cryalg_DIGITS] & 0x01) << 63;
        vli_rshift1(l_modMultiple + NUM_cryalg_DIGITS);
        vli_rshift1(l_modMultiple);
        l_modMultiple[NUM_cryalg_DIGITS-1] |= l_carry;
        
        --l_productBits;
    }
    vli_set(p_result, l_product);
}

static uint umax(uint a, uint b)
{
    return (a > b ? a : b);
}


int cryalgsa_verify(const uint8_t p_publicKey[cryalg_BYTES+1], const uint8_t p_hash[cryalg_BYTES], const uint8_t p_signature[cryalg_BYTES*2])
{
    uint64_t u1[NUM_cryalg_DIGITS], u2[NUM_cryalg_DIGITS];
    uint64_t z[NUM_cryalg_DIGITS];
    cryalgPoint l_public, l_sum;
    uint64_t rx[NUM_cryalg_DIGITS];
    uint64_t ry[NUM_cryalg_DIGITS];
    uint64_t tx[NUM_cryalg_DIGITS];
    uint64_t ty[NUM_cryalg_DIGITS];
    uint64_t tz[NUM_cryalg_DIGITS];
    
    uint64_t l_r[NUM_cryalg_DIGITS], l_s[NUM_cryalg_DIGITS];
    
    cryalg_point_decompress(&l_public, p_publicKey);
    cryalg_bytes2native(l_r, p_signature);
    cryalg_bytes2native(l_s, p_signature + cryalg_BYTES);
    
    if(vli_isZero(l_r) || vli_isZero(l_s)){ 
        return 0;
    }
    
    if(vli_cmp(curve_n, l_r) != 1 || vli_cmp(curve_n, l_s) != 1){ 
        return 0;
    }

    vli_modInv(z, l_s, curve_n); 
    cryalg_bytes2native(u1, p_hash);
    vli_modMult(u1, u1, z, curve_n); 
    vli_modMult(u2, l_r, z, curve_n); 
    
    vli_set(l_sum.x, l_public.x);
    vli_set(l_sum.y, l_public.y);
    vli_set(tx, curve_G.x);
    vli_set(ty, curve_G.y);
    vli_modSub(z, l_sum.x, tx, curve_p); 
    XYcZ_add(tx, ty, l_sum.x, l_sum.y);
    vli_modInv(z, z, curve_p); 
    apply_z(l_sum.x, l_sum.y, z);
    
    cryalgPoint *l_points[4] = {NULL, &curve_G, &l_public, &l_sum};
    uint l_numBits = umax(vli_numBits(u1), vli_numBits(u2));
    
    cryalgPoint *l_point = l_points[(!!vli_testBit(u1, l_numBits-1)) | ((!!vli_testBit(u2, l_numBits-1)) << 1)];
    vli_set(rx, l_point->x);
    vli_set(ry, l_point->y);
    vli_clear(z);
    z[0] = 1;

    int i;
    for(i = l_numBits - 2; i >= 0; --i){
        cryalgPoint_double_jacobian(rx, ry, z);
        
        int l_index = (!!vli_testBit(u1, i)) | ((!!vli_testBit(u2, i)) << 1);
        cryalgPoint *l_point = l_points[l_index];
        if(l_point){
            vli_set(tx, l_point->x);
            vli_set(ty, l_point->y);
            apply_z(tx, ty, z);
            vli_modSub(tz, rx, tx, curve_p); 
            XYcZ_add(tx, ty, rx, ry);
            vli_modMult_fast(z, z, tz);
        }
    }

    vli_modInv(z, z, curve_p); 
    apply_z(rx, ry, z);
    
    if(vli_cmp(curve_n, rx) != 1){
        vli_sub(rx, rx, curve_n);
    }

    return (vli_cmp(rx, l_r) == 0);
}
