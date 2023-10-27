#include <fstream>
#include <iostream>
#include <mutex>
#include <sodium.h>
#include <sstream>

#include "../constants/constants.h"
#include "../crypto/encryption_engine.h"
#include "../gen-cpp/RPC.h"
#include "../host/redis.h"
#include <argparse/argparse.hpp>

struct ClientConfig {
    std::ifstream seed_data;
    int num_clients = 16;
    int num_operations = 1000;
    double p_get = 0.5;
    bool init_db = false;

    int max_key = 100000;
    int max_value = 100000;
};

bool moreOperationsExist(ClientConfig &config);

Operation getOperation(ClientConfig &config);

Operation genRandOperation(ClientConfig &config);

Operation getSeedOperation(ClientConfig &config);

std::istream &readFile(std::ifstream &seed_data, std::string &line);

std::string clientEncrypt(const std::string &value);

void parseArgs(int argc, char *argv[], ClientConfig &config);