#include <iostream>

#include "../crypto/encryption_engine.h"
#include "../host/redis.h"
#include "spdlog/spdlog.h"

using namespace std;

int main(int argc, char *argv[]) {
    encryption_engine encryption_engine_;
    redisCli rd;
    unsigned char cipher_text[4096];
    auto encLen = encryption_engine_.encryptNonDeterministic("30", cipher_text);
    string val((const char *)cipher_text, encLen);
    spdlog::info("Encrypted value is: {0}", val);
    rd.put("1", val);
    spdlog::info("Redis get encrypted for key 1: {0}", rd.get("1"));
    spdlog::info("Decrypted value for key 1: {0}", encryption_engine_.decryptNonDeterministic(rd.get("1")));

    encLen = encryption_engine_.encryptNonDeterministic("20", cipher_text);
    string val1((const char *)cipher_text, encLen);
    rd.put("2", val1);

    spdlog::info("Redis get encrypted for key 2: {0}", rd.get("2"));
    spdlog::info("Decrypted value for key 2: {0}", encryption_engine_.decryptNonDeterministic(rd.get("2")));
    return 0;
}