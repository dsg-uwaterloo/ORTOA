#ifndef WAFFLE_ENCRYPTION_ENGINE_H
#define WAFFLE_ENCRYPTION_ENGINE_H

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <assert.h>
#include <string>
#include <random>
#include <algorithm>
#include <stdint.h>
#include <chrono>
#include <ctime>

#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>

typedef unsigned char byte;
#define UNUSED(x) ((void)x)
const char hn[] = "SHA256";

class encryption_engine {
public:
    encryption_engine();
    int encrypt(const std::string &plain_text, unsigned char* cipher_text);
    std::string decrypt(const std::string &cipher_text);
    std::string hmac(const std::string &key);
    int encryptNonDeterministic(const std::string &plain_text, unsigned char* cipher_text);
    std::string decryptNonDeterministic(const std::string &cipher_text);
    std::string getencryption_string_();
    std::string extractKey(const std::string &encryptedKey);
    std::string prf(const std::string &plain_text);
    uint32_t rand_uint32(const uint32_t &min, const uint32_t &max);
    std::string rand_str(const int len);

private:
    void handle_errors();
    int sign_it(const byte* msg, size_t mlen, byte** sig, size_t* slen, EVP_PKEY* pkey);
    int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
                unsigned char *iv, unsigned char *ciphertext);
    int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
                unsigned char *iv, unsigned char *plaintext);
    int verify_it(const byte* msg, size_t mlen, const byte* sig, size_t slen, EVP_PKEY* pkey);
    void print_it(const char* label, const byte* buff, size_t len);
    int make_keys(EVP_PKEY** skey, EVP_PKEY** vkey);
    int hmac_it(const byte* msg, size_t mlen, byte** val, size_t* vlen, EVP_PKEY* pkey);

    std::string encryption_string_;
    std::string iv_string_;
    unsigned char * encryption_key_;
    unsigned char * iv_;
    EVP_PKEY * skey_, * vkey_;
    std::string prf_encrypt(const std::string& key, const std::string& plaintext);

    int rng_max_len = 10;
};

#endif // WAFFLE_BASIC_CRYPTO_H
