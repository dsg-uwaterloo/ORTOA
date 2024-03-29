#ifndef SHARED_QUEUE_H
#define SHARED_QUEUE_H

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

#include <chrono>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "RPC.h"
#include "clientUtils.h"
#include "constants.h"

using namespace std::chrono;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

class SharedQueue {
  private:
    std::mutex mutex;
    std::queue<Operation> queue;

    ClientConfig &config;

  public:
    SharedQueue(ClientConfig &config) : config(config){};

    int enqueue() {
        std::unique_lock<std::mutex> lock(mutex);

        if (!moreOperationsExist(config)) {
            return 1;
        }

        queue.push(getOperation(config));

        return 0;
    }

    Operation dequeue() {
        std::unique_lock<std::mutex> lock(mutex);

        if (queue.empty()) {
            Operation op;
            op.__set_op(OpType::EOD);
            return op;
        }

        Operation data = queue.front();
        queue.pop();

        return data;
    }
};

class DataHandler {
  private:
    SharedQueue &sharedQueue;

  public:
    DataHandler(SharedQueue &sharedQueue) : sharedQueue(sharedQueue) {}

    void operator()() {
        while (true) {
            int enqueue_result = sharedQueue.enqueue();
            if (enqueue_result == 1)
                return;
        }
    }
};

class WarmUpRunner {
  private:
    SharedQueue &sharedQueue;
    inline static std::mutex mutex;

  public:
    inline static std::atomic<int> warmupOperations;

    WarmUpRunner(SharedQueue &sharedQueue) : sharedQueue(sharedQueue) {}

    void operator()() {
        auto socket = std::make_shared<TSocket>(HOST_IP, HOST_PORT);
        auto transport = std::make_shared<TBufferedTransport>(socket);
        auto protocol = std::make_shared<TBinaryProtocol>(transport);
        RPCClient client(protocol);

        transport->open();

        while (warmupOperations--) {
            Operation data = sharedQueue.dequeue();
            if (data.op == OpType::EOD)
                return;

            std::string out;
            client.access(out, data);
        }

        transport->close();
    }
};

class ClientRunner {
  private:
    SharedQueue &sharedQueue;
    std::vector<double> &latencies;

  public:
    ClientRunner(SharedQueue &sharedQueue, std::vector<double> &latencies)
        : sharedQueue(sharedQueue), latencies(latencies) {}

    void operator()() {
        auto socket = std::make_shared<TSocket>(HOST_IP, HOST_PORT);
        auto transport = std::make_shared<TBufferedTransport>(socket);
        auto protocol = std::make_shared<TBinaryProtocol>(transport);
        RPCClient client(protocol);

        transport->open();

        while (true) {
            Operation data = sharedQueue.dequeue();
            if (data.op == OpType::EOD)
                return;

            auto start = high_resolution_clock::now();
            std::string out;
            client.access(out, data);
            auto end = high_resolution_clock::now();
            latencies.push_back(duration_cast<milliseconds>(end - start).count());
        }

        transport->close();
    }
};

#endif
