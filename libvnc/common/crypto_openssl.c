/*
 * crypto_openssl.c - Crypto wrapper (openssl version)
 */

/*
 *  Copyright (C) 2011 Gernot Tenchio
 *  Copyright (C) 2019 Christian Beier
 *
 *  This is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 *  USA.
 */

#include <string.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <openssl/dh.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include "crypto.h"

static unsigned char reverseByte(unsigned char b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

int hash_md5(void *out, const void *in, const size_t in_len)
{
    EVP_MD_CTX *mdctx;
    unsigned int md5_digest_len = MD5_HASH_SIZE;

    mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL) return 0;
    if (!EVP_DigestInit_ex(mdctx, EVP_md5(), NULL)) {
        EVP_MD_CTX_free(mdctx);
        return 0;
    }

    if (!EVP_DigestUpdate(mdctx, in, in_len)) {
        EVP_MD_CTX_free(mdctx);
        return 0;
    }

    if (!EVP_DigestFinal_ex(mdctx, out, &md5_digest_len)) {
        EVP_MD_CTX_free(mdctx);
        return 0;
    }

    EVP_MD_CTX_free(mdctx);

    return 1;
}

int hash_sha1(void *out, const void *in, const size_t in_len)
{
    EVP_MD_CTX *mdctx;
    unsigned int digest_len = SHA1_HASH_SIZE;

    mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL) return 0;
    if (!EVP_DigestInit_ex(mdctx, EVP_sha1(), NULL)) {
        EVP_MD_CTX_free(mdctx);
        return 0;
    }

    if (!EVP_DigestUpdate(mdctx, in, in_len)) {
        EVP_MD_CTX_free(mdctx);
        return 0;
    }

    if (!EVP_DigestFinal_ex(mdctx, out, &digest_len)) {
        EVP_MD_CTX_free(mdctx);
        return 0;
    }

    EVP_MD_CTX_free(mdctx);

    return 1;
}

void random_bytes(void *out, size_t len)
{
    RAND_bytes(out, len);
}

int encrypt_rfbdes(void *out, int *out_len, const unsigned char key[8], const void *in, const size_t in_len)
{
    int result = 0;
    EVP_CIPHER_CTX *des;
    unsigned char mungedkey[8];
    int i;

    for (i = 0; i < 8; i++)
      mungedkey[i] = reverseByte(key[i]);

    if(!(des = EVP_CIPHER_CTX_new()))
	goto out;
    if(!EVP_EncryptInit_ex(des, EVP_des_ecb(), NULL, mungedkey, NULL))
	goto out;
    if(!EVP_EncryptUpdate(des, out, out_len, in, in_len))
	goto out;

    result = 1;

 out:
    EVP_CIPHER_CTX_free(des);
    return result;
}

int decrypt_rfbdes(void *out, int *out_len, const unsigned char key[8], const void *in, const size_t in_len)
{
    int result = 0;
    EVP_CIPHER_CTX *des;
    unsigned char mungedkey[8];
    int i;

    for (i = 0; i < 8; i++)
      mungedkey[i] = reverseByte(key[i]);

    if(!(des = EVP_CIPHER_CTX_new()))
	goto out;
    if(!EVP_DecryptInit_ex(des, EVP_des_ecb(), NULL, mungedkey, NULL))
	goto out;
    if(!EVP_DecryptUpdate(des, out, out_len, in, in_len))
	goto out;

    result = 1;

 out:
    EVP_CIPHER_CTX_free(des);
    return result;
}

int encrypt_aes128ecb(void *out, int *out_len, const unsigned char key[16], const void *in, const size_t in_len)
{
    int result = 0;
    EVP_CIPHER_CTX *aes;

    if(!(aes = EVP_CIPHER_CTX_new()))
	goto out;
    EVP_CIPHER_CTX_set_padding(aes, 0);
    if(!EVP_EncryptInit_ex(aes, EVP_aes_128_ecb(), NULL, key, NULL))
	goto out;
    if(!EVP_EncryptUpdate(aes, out, out_len, in, in_len))
	goto out;

    result = 1;

 out:
    EVP_CIPHER_CTX_free(aes);
    return result;
}

int dh_generate_keypair(uint8_t *priv_out, uint8_t *pub_out, const uint8_t *gen, const size_t gen_len, const uint8_t *prime, const size_t keylen)
{
    int result = 0;
    DH *dh;

    if(!(dh = DH_new()))
	goto out;
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined LIBRESSL_VERSION_NUMBER
    dh->p = BN_bin2bn(prime, keylen, NULL);
    dh->g = BN_bin2bn(gen, gen_len, NULL);
#else
    if(!DH_set0_pqg(dh, BN_bin2bn(prime, keylen, NULL), NULL, BN_bin2bn(gen, gen_len, NULL)))
	goto out;
#endif
    if(!DH_generate_key(dh))
	goto out;
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined LIBRESSL_VERSION_NUMBER
    if(BN_bn2bin(dh->priv_key, priv_out) == 0)
	goto out;
    if(BN_bn2bin(dh->pub_key, pub_out) == 0)
	goto out;
#else
    if(BN_bn2binpad(DH_get0_priv_key(dh), priv_out, keylen) == -1)
	goto out;
    if(BN_bn2binpad(DH_get0_pub_key(dh), pub_out, keylen) == -1)
	goto out;
#endif

    result = 1;

 out:
    DH_free(dh);
    return result;
}

int dh_compute_shared_key(uint8_t *shared_out, const uint8_t *priv, const uint8_t *pub, const uint8_t *prime, const size_t keylen)
{
    int result = 0;
    DH *dh;

    if(!(dh = DH_new()))
	goto out;
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined LIBRESSL_VERSION_NUMBER
    dh->p = BN_bin2bn(prime, keylen, NULL);
    dh->priv_key = BN_bin2bn(priv, keylen, NULL);
#else
    if(!DH_set0_pqg(dh, BN_bin2bn(prime, keylen, NULL), NULL, BN_new()))
	goto out;
    if(!DH_set0_key(dh, NULL, BN_bin2bn(priv, keylen, NULL)))
	goto out;
#endif
    if(DH_compute_key(shared_out, BN_bin2bn(pub, keylen, NULL), dh) == -1)
	goto out;

    result = 1;

 out:
    DH_free(dh);
    return result;
}
