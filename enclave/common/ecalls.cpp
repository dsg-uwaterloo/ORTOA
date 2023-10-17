// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/enclave.h>

#include <iostream>
#include <string>

#include "../crypto/encryption_engine.h"
#include "ortoa_t.h"
#include "shared.h"
using namespace std;

void access_data(const char* opConst, size_t opConstSize,
                 const char* inVal, size_t inSize, unsigned char* cipher_text, size_t* outSize) {
    int c, val, out;
    string cStr = opConst;
    encryption_engine encryption_engine_;
    string inStr((const char*)inVal, inSize);
    string valStr = encryption_engine_.decryptNonDeterministic(inStr);
    c = stoi(cStr);
    val = stoi(valStr);

    cout << "In enclave: Decryption val is: " << valStr << endl;
    // TODO: If c == 0: read else write
    out = c + val;
    *outSize = (size_t)encryption_engine_.encryptNonDeterministic(to_string(out), cipher_text);
}
