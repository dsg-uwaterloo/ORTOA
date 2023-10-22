#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <sodium.h>

#include "../constants/constants.h"
#include "../crypto/encryption_engine.h"
#include "../gen-cpp/RPC.h"
#include "../host/redis.h"

Operation genRandOperation();

Operation getSeedOperation(std::string& line);

std::istream& readFile(std::ifstream &seed_data, std::string &line);

std::string clientEncrypt(const std::string &value);
