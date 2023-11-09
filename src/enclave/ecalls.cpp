// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <iostream>
#include <openenclave/enclave.h>
#include <string>

#include "constants/shared.h"
#include "crypto/encryption_engine.h"
#include "ortoa_t.h"

void access_data(int op_const, const char *in_val, size_t in_size,
                 const char *update_val, size_t update_size,
                 unsigned char *cipher_text, size_t *out_size) {
    encryption_engine engine;

    std::cout << "[Enclave]: 1" << std::endl;

    // Decrypt value from redis
    std::string in_str((const char *)in_val, in_size);
    std::cout << "[Enclave]: 2" << std::endl;
    std::string val_decrypt = engine.decryptNonDeterministic(in_str);
    std::cout << "[Enclave]: 3" << std::endl;

    // Decrypt update value from client
    std::string update_str((const char *)update_val, update_size);
    std::string u_val_decrypt = engine.decryptNonDeterministic(update_str);
    std::cout << "[Enclave]: 4" << std::endl;

    // std::cout << "[Enclave]: Decrypted value is: " << val_decrypt <<
    // std::endl; std::cout << "[Enclave]: Decrypted update value is: " <<
    // u_val_decrypt
    //           << std::endl;

    // If operation is GET then re-encrypt the value fetched from redis,
    // otherwise, encrypt the update value from client
    std::string value = (op_const == 0) ? val_decrypt : u_val_decrypt;

    std::cout << "[Decrypted val chosen]: " << value << std::endl;
    *out_size = engine.encryptNonDeterministic(value, cipher_text);
    std::cout << "[Enclave]: 5" << std::endl;
}