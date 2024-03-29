#ifndef CLIENT_HELPER_H
#define CLIENT_HELPER_H

#include <set>
#include <string>
#include <cstring>
#include <utility>
#include <iostream>
#include <unordered_map>
#include "BS_thread_pool.hpp"


#include <sodium.h>

#include "gen-cpp/KV_RPC.h"

#include "constants.h"

#define CIPHERTEXT_LEN (crypto_secretbox_MACBYTES + crypto_secretbox_KEYBYTES + 1)

#define CONTEXT "OpScureK"

extern std::set<std::string> keySet;
extern std::unordered_map<std::string, int> valueSizes;
extern std::unordered_map<std::string, std::string> masterKeys;
extern std::unordered_map<std::string, std::atomic<bool>> locks;
extern BS::thread_pool* pool;



extern unsigned char randomBytes[VALUE_SIZE];

void OpScureSetup(std::string fileName);
void OpScureCleanup(std::string fileName);

Entry constructCreateEntry(std::string& key, std::string& value);
Entry constructGetEntry(std::string& key);
Entry constructPutEntry(std::string& key, std::string& value);

std::string readValueFromLabels(std::string key, std::string labels);

#endif
