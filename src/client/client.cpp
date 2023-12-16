#include <chrono>
#include <numeric>
#include <sstream>

#include "redis.h"
#include "SharedQueue.h"
#include "spdlog/spdlog.h"

using namespace std::chrono;

class ClientHandler {
  private:
    ClientConfig config;
    std::vector<double> latencies;
    double total_duration;

  public:
    ClientHandler(int argc, char *argv[]) { parseArgs(argc, argv, config); }

    void start() { (config.init_db) ? initDB() : runThreaded(); }

    void initDB() {
        // # of operations corresponds to max_key (if seed data is not used)
        config.num_operations = config.max_key;

        redisCli rd;
        auto pipeline = rd.pipe();

        while (moreOperationsExist(config)) {
            Operation op = getInitKV(config);
            pipeline.set(op.key, op.value);
        }
        pipeline.exec();
    }

    void runThreaded() {
        SharedQueue sharedQueue(config);
        std::vector<std::thread> data_handler_threads;
        std::vector<std::thread> runner_threads;

        // Data streaming into a shared queue
        for (int i = 0; i < config.num_clients; ++i) {
            data_handler_threads.push_back(std::thread(DataHandler(sharedQueue)));
        }

        for (auto& thread : data_handler_threads) thread.join();

        // Client data access using shared queue
        auto start = high_resolution_clock::now();
        for (int i = 0; i < config.num_clients; ++i) {
            runner_threads.push_back(std::thread(ClientRunner(sharedQueue, latencies)));
        }

        for (auto& thread : runner_threads) thread.join();
        auto end = high_resolution_clock::now();

        total_duration = duration_cast<microseconds>(end - start).count();
    }

    float getAveLatency() {
        assert(latencies.size() > 0);

        auto average_latency =
            std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();

        spdlog::info("[Client]: Data access complete, average latency: {0} microseconds", average_latency);
        return average_latency;
    }

    float getTotalDuration() {
        assert(total_duration > 0);

        spdlog::info("[main]: Entire program finished in {0} microseconds", total_duration);
        return total_duration;
    }

    void writeOutput() {
        if (config.init_db) {
            return;
        }

        if (!config.experiment_result_file.is_open()) {
            getAveLatency();
            getTotalDuration();
            return;
        }

        for (auto l : latencies) {
            config.experiment_result_file << l << ",";
        }

        config.experiment_result_file << std::endl;
        config.experiment_result_file << getAveLatency() << std::endl;
        config.experiment_result_file << getTotalDuration() << std::endl;
        config.experiment_result_file.flush();
    }
};

int main(int argc, char *argv[]) {
    try {
        ClientHandler client(argc, argv);

        client.start();
        client.writeOutput();
    } catch (std::runtime_error err) {
        spdlog::error("Client | {0}", err.what());
    } catch (TException &err) {
        spdlog::error("Client | {0}", err.what());
    }
}
