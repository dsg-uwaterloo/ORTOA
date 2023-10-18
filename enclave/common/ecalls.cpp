// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <iostream>
#include <openenclave/enclave.h>
#include <string>

#include "constants/shared.h"
#include "crypto/encryption_engine.h"
#include "ortoa_t.h"

using namespace std;

void access_data(int op_const, const char* in_val, size_t in_size, const char* update_val, size_t update_size, unsigned char* cipher_text, size_t* out_size) {
    encryption_engine engine;

    // Decrypt value from redis
    string in_str((const char *) in_val, in_size);
    string val_decrypt = engine.decryptNonDeterministic(in_str);

    // Decrypt update value from client
    string update_str((const char *) update_val, update_size);
    string u_val_decrypt = engine.decryptNonDeterministic(update_str);

    cout << "[Enclave]: Decrypted value is: " << val_decrypt << endl;
    cout << "[Enclave]: Decrypted update value is: " << u_val_decrypt << endl;
    
    // If operation is GET then re-encrypt the value fetched from redis, 
    // otherwise, encrypt the update value from client
    string value = (op_const == 0) ? val_decrypt : u_val_decrypt;
    *out_size = (size_t)engine.encryptNonDeterministic(value, cipher_text);
}