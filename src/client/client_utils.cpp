#include "client_utils.h"

std::mutex fileMutex;

bool moreOperationsExist(ClientConfig &config) {
    return (config.use_seed && !config.seed_data.eof()) || 
           (!config.use_seed && config.num_operations > 0);
}

Operation getInitKV(ClientConfig &config) {
    if (config.use_seed) {
        return getSeedOperation(config);
    } else {
        return genRandInitValue(config);
    }
}

Operation getOperation(ClientConfig &config) {
    if (config.use_seed) {
        return getSeedOperation(config);
    } else {
        return genRandOperation(config);
    }
}

Operation getSeedOperation(ClientConfig &config) {
    std::string line, operation, key, value;

    std::getline(config.seed_data, line);
    std::istringstream ss(line);
    ss >> operation >> key >> value;

    Operation op;
    op.__set_op((operation == "GET") ? OpType::GET : OpType::PUT);
    op.__set_key(key);

    // If operation is GET, then update value to random bytes
    if (op.op == OpType::GET) {
        char rand_val[VALUE_SIZE];
        randombytes_buf(rand_val, VALUE_SIZE);
        value = std::string(rand_val);
    }

    op.__set_value(clientEncrypt(value));
    return op;
}

Operation genRandInitValue(ClientConfig &config) {
    Operation op;

    std::string value = std::to_string(rand() % config.max_value);
    op.__set_key(std::to_string(config.max_key - config.num_operations));
    op.__set_value(clientEncrypt(value));

    // Decrement config.num_operations
    --config.num_operations;
    return op;
}

Operation genRandOperation(ClientConfig &config) {
    double r = (double)rand() / RAND_MAX;
    int key = rand() % config.max_key;

    Operation op;
    op.__set_op(r < config.p_get ? OpType::GET : OpType::PUT);
    op.__set_key(std::to_string(key));

    std::string value;

    // If operation is GET, then set value to random bytes
    // If operation is PUT, then set value to random int value
    if (op.op == OpType::GET) {
        char rand_val[VALUE_SIZE];
        randombytes_buf(rand_val, VALUE_SIZE);
        value = std::string(rand_val);
    } else {
        int put_val = rand() % config.max_value;
        value = std::to_string(put_val);
    }
    op.__set_value(clientEncrypt(value));

    // Decrement config.num_operations
    --config.num_operations;
    return op;
}

std::string clientEncrypt(const std::string &value) {
    encryption_engine engine;

    std::unique_ptr<unsigned char> cipher_text(new unsigned char[4096]);
    size_t out_len =
        (size_t)engine.encryptNonDeterministic(value, cipher_text.get());
    std::string updated_val((const char *)cipher_text.get(), out_len);
    return updated_val;
}

void parseArgs(int argc, char *argv[], ClientConfig &config) {
    argparse::ArgumentParser program("ortoa-tee");

    program.add_argument("--seed").default_value(std::string{""});

    program.add_argument("-o", "--output").default_value(std::string{""});

    program.add_argument("--nthreads").default_value(16).scan<'d', int>();

    program.add_argument("--warmup").default_value(100).scan<'d', int>();

    program.add_argument("--noperations").default_value(1000).scan<'d', int>();

    program.add_argument("--initdb").default_value(false).implicit_value(true);

    program.add_argument("--pget").default_value(0.5).scan<'g', double>();

    program.add_argument("--max-key").default_value(100000).scan<'d', int>();

    program.add_argument("--max-val").default_value(100000).scan<'d', int>();

    program.parse_args(argc, argv);

    if (program.is_used("--seed")) {
        auto seed_path = program.get<std::string>("--seed");
        config.seed_data.open(seed_path);

        if (!config.seed_data.is_open()) {
            throw std::runtime_error("Invalid path to seed data");
        }

        config.use_seed = true;
    }

    if (program.is_used("--output")) {
        auto output_path = program.get<std::string>("--output");
        config.experiment_result_file.open(output_path);

        if (!config.experiment_result_file.is_open()) {
            throw std::runtime_error("Invalid path to experiment result file");
        }
    } 

    config.num_clients = program.get<int>("--nthreads");
    config.num_warmup_operations = program.get<int>("--warmup");
    config.num_operations = program.get<int>("--noperations");
    config.init_db = program.get<bool>("--initdb");
    config.p_get = program.get<double>("--pget");
    config.max_key = program.get<int>("--max-key");
    config.max_value = program.get<int>("--max-val");
}
