// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include <string>

#include "shared.h"
#include "encryption_engine.h"
#include "ortoa_t.h"

void access_data(int op_const, const char *in_val, size_t in_size,
                 const char *update_val, size_t update_size,
                 unsigned char *cipher_text, size_t *out_size) {
    encryption_engine engine;

    // If operation is GET then re-encrypt the value fetched from redis
    // Else operation is PUT then encrypt the update value from client
    if (op_const == 0) {
        // Decrypt value from redis
        std::string in_str(in_val, in_size);
        std::string val_decrypt = engine.decryptNonDeterministic(in_str);

        *out_size = engine.encryptNonDeterministic(val_decrypt, cipher_text);
    } else {
        // Decrypt update value from client
        std::string update_str(update_val, update_size);
        std::string u_val_decrypt = engine.decryptNonDeterministic(update_str);

        *out_size = engine.encryptNonDeterministic(u_val_decrypt, cipher_text);
    }
}
