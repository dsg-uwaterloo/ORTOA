#include <chrono>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sodium.h>
#include <sstream>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "../constants/constants.h"
#include "../crypto/encryption_engine.h"
#include "../gen-cpp/RPC.h"
#include "../host/redis.h"
#include "client_utils.h"

using namespace std::chrono;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

class ClientHandler {
  private:
    std::ifstream seed_data;
    std::ofstream experiment_result_file;
    bool init_db = false;
    int num_clients = 16;
    float p_get = 0.5;

    std::vector<float> latencies;

  public:
    ClientHandler(int argc, char *argv[]) {
        parseArgs(argc, argv, seed_data, init_db, num_clients, p_get,
                  experiment_result_file);
    }

    void start() {
        if (init_db) {
            initDB();
        } else {
            runThreaded();
        }
    }

    void initDB() {
        redisCli rd;
        auto pipeline = rd.pipe();

        // If seed data exists, initialize the db with seed data
        if (seed_data.is_open()) {
            std::string line;
            while (std::getline(seed_data, line)) {
                Operation op = getSeedOperation(line);
                pipeline.set(op.key, op.value);
            }
        }
        // If seed data does not exist, initialize db with key from 0 to KEY_MAX
        else {
            for (int i = 0; i < KEY_MAX; ++i) {
                std::string value = std::to_string(rand() % VAL_MAX);
                pipeline.set(std::to_string(i), clientEncrypt(value));
            }
        }

        pipeline.exec();
    }

    void runThreaded() {
        std::vector<std::thread> threads;
        for (int i = 0; i < num_clients; i++) {
            threads.push_back(std::thread(&ClientHandler::run, this));
        }

        // Wait for all threads to finish
        for (std::thread &thread : threads)
            thread.join();

        getAveLatency();
        writeOutput();
    }

    void run() {
        auto socket = std::make_shared<TSocket>(HOST_IP, HOST_PORT);
        auto transport = std::make_shared<TBufferedTransport>(socket);
        auto protocol = std::make_shared<TBinaryProtocol>(transport);
        RPCClient client(protocol);

        transport->open();

        std::string val;
        // If seed data exists, run the client with data
        if (seed_data.is_open()) {
            std::string line;
            while (readFile(seed_data, line)) {
                Operation op = getSeedOperation(line);
                auto start = high_resolution_clock::now();
                client.access(val, op);
                auto end = high_resolution_clock::now();
                latencies.push_back(
                    duration_cast<microseconds>(end - start).count());
            }
        }
        // If seed data does not exist, run client on random values
        else {
            for (int i = 0; i < 1000; ++i) {
                Operation op = genRandOperation(p_get);
                auto start = high_resolution_clock::now();
                client.access(val, op);
                auto end = high_resolution_clock::now();
                latencies.push_back(
                    duration_cast<microseconds>(end - start).count());
            }
        }

        transport->close();
    }

    float getAveLatency() {
        assert(latencies.size() > 0);
        auto average_latency =
            std::accumulate(latencies.begin(), latencies.end(), 0.0) /
            latencies.size();
        std::cout << "[Client]: Data access complete, average latency: "
                  << average_latency << " microseconds" << std::endl;
    }

    void writeOutput() {
        for (auto l : latencies) {
            experiment_result_file << l << ",";
        }
        experiment_result_file << std::endl;

        experiment_result_file << getAveLatency() << std::endl;
    }
};

int main(int argc, char *argv[]) {
    try {
        ClientHandler client(argc, argv);

        auto start = high_resolution_clock::now();
        client.start();
        auto end = high_resolution_clock::now();

        std::cout << "[main]: Entire program finished in "
                  << duration_cast<microseconds>(end - start).count()
                  << " microseconds" << std::endl;
    } catch (std::invalid_argument &err) {
        std::cerr << "ERROR: " << err.what() << std::endl;
    } catch (TException &err) {
        std::cerr << "ERROR: " << err.what() << std::endl;
    }
}
