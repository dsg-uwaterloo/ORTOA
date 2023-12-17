#include <fstream>
#include <mutex>
#include <sodium.h>
#include <sstream>
#include <argparse/argparse.hpp>

#include "../constants/constants.h"
#include "../crypto/encryption_engine.h"
#include "../gen-cpp/RPC.h"
#include "redis.h"

struct ClientConfig {
    std::ifstream seed_data;
    std::ofstream experiment_result_file;

    int num_clients = 16;
    int num_warmup_operations = 100;
    int num_operations = 1000;
    double p_get = 0.5;

    bool init_db = false;
    bool use_seed = false;

    int max_key = 100000;
    int max_value = 10000;
};

bool moreOperationsExist(ClientConfig &config);

Operation getInitKV(ClientConfig &config);

Operation getOperation(ClientConfig &config);

Operation getSeedOperation(ClientConfig &config);

Operation genRandInitValue(ClientConfig &config);

Operation genRandOperation(ClientConfig &config);

std::string clientEncrypt(const std::string &value);

void parseArgs(int argc, char *argv[], ClientConfig &config);
