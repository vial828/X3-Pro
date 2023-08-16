
#include "stdint.h"

typedef struct
{
    uint32_t state[5];
    uint32_t count[2];
    unsigned char buffer[64];
} hashalg_CTX;

void hashalgTransform(
    uint32_t state[5],
    const unsigned char buffer[64]
    );

void hashalgInit(
    hashalg_CTX * context
    );

void hashalgUpdate(
    hashalg_CTX * context,
    const unsigned char *data,
    uint32_t len
    );

void hashalgFinal(
    unsigned char digest[20],
    hashalg_CTX * context
    );

void hashalg(
    char *hash_out,
    const char *str,
    int len);

#define cryalg_CURVE 16


#define cryalg_BYTES cryalg_CURVE

/* cryalg_make_key() function.
Create a public/private key pair.
    
Outputs:
    p_publicKey  - Will be filled in with the public key.
    p_privateKey - Will be filled in with the private key.

Returns 1 if the key pair was generated successfully, 0 if an error occurred.
*/
int cryalg_make_key(uint8_t p_publicKey[cryalg_BYTES+1], uint8_t p_privateKey[cryalg_BYTES]);

/* cryalgh_shared_secret() function.
Compute a shared secret given your secret key and someone else's public key.
Note: It is recommended that you hash the result of cryalgh_shared_secret before using it for symmetric encryption or HMAC.

Inputs:
    p_publicKey  - The public key of the remote party.
    p_privateKey - Your private key.

Outputs:
    p_secret - Will be filled in with the shared secret value.

Returns 1 if the shared secret was generated successfully, 0 if an error occurred.
*/
int cryalgh_shared_secret(const uint8_t p_publicKey[cryalg_BYTES+1], const uint8_t p_privateKey[cryalg_BYTES], uint8_t p_secret[cryalg_BYTES]);

/* cryalgsa_sign() function.
Generate an cryalgSA signature for a given hash value.

Usage: Compute a hash of the data you wish to sign (SHA-2 is recommended) and pass it in to
this function along with your private key.

Inputs:
    p_privateKey - Your private key.
    p_hash       - The message hash to sign.

Outputs:
    p_signature  - Will be filled in with the signature value.

Returns 1 if the signature generated successfully, 0 if an error occurred.
*/
int cryalgsa_sign(const uint8_t p_privateKey[cryalg_BYTES], const uint8_t p_hash[cryalg_BYTES], uint8_t p_signature[cryalg_BYTES*2]);

/* cryalgsa_verify() function.
Verify an cryalgSA signature.

Usage: Compute the hash of the signed data using the same hash as the signer and
pass it to this function along with the signer's public key and the signature values (r and s).

Inputs:
    p_publicKey - The signer's public key
    p_hash      - The hash of the signed data.
    p_signature - The signature value.

Returns 1 if the signature is valid, 0 if it is invalid.
*/
int cryalgsa_verify(const uint8_t p_publicKey[cryalg_BYTES+1], const uint8_t p_hash[cryalg_BYTES], const uint8_t p_signature[cryalg_BYTES*2]);
