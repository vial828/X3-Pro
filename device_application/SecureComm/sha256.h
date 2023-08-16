/* Sha256.h -- SHA-256 Hash
2010-06-11 : Igor Pavlov : Public domain */

#ifndef __CRYPTO_SHA256_H
#define __CRYPTO_SHA256_H

#include <stdlib.h>
#include <stdint.h>

#ifdef _MSC_VER

#include <stdlib.h>

#define ROTL32(v, n) _rotl((v), (n))
#define ROTL64(v, n) _rotl64((v), (n))

#define ROTR32(v, n) _rotr((v), (n))
#define ROTR64(v, n) _rotr64((v), (n))

#else

#include <stdint.h>

#define U8V(v) ((uint8_t)(v) & 0xFFU)
#define U16V(v) ((uint16_t)(v) & 0xFFFFU)
#define U32V(v) ((uint32_t)(v) & 0xFFFFFFFFU)
#define U64V(v) ((uint64_t)(v) & 0xFFFFFFFFFFFFFFFFU)

#define ROTL32(v, n) \
  (U32V((uint32_t)(v) << (n)) | ((uint32_t)(v) >> (32 - (n))))

// tests fail if we don't have this cast...
#define ROTL64(v, n) \
  (U64V((uint64_t)(v) << (n)) | ((uint64_t)(v) >> (64 - (n))))

#define ROTR32(v, n) ROTL32(v, 32 - (n))
#define ROTR64(v, n) ROTL64(v, 64 - (n))

#endif

#define ROTL8(v, n) \
  (U8V((uint8_t)(v) << (n)) | ((uint8_t)(v) >> (8 - (n))))

#define ROTL16(v, n) \
  (U16V((uint16_t)(v) << (n)) | ((uint16_t)(v) >> (16 - (n))))

#define ROTR8(v, n) ROTL8(v, 8 - (n))
#define ROTR16(v, n) ROTL16(v, 16 - (n))

#define SHA256_DIGEST_SIZE 32

typedef struct sha256_t
{
  uint32_t state[8];
  uint64_t count;
  unsigned char buffer[64];
} sha256_t;

void sha256_init(sha256_t *p);
void sha256_update(sha256_t *p, const unsigned char *data, size_t size);
void sha256_final(sha256_t *p, unsigned char *digest);
void sha256_hash(unsigned char *buf, const unsigned char *data, size_t size);

#endif
