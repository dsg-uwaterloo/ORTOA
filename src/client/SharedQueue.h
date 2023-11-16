#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

#include <chrono>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "../constants/constants.h"
#include "../gen-cpp/RPC.h"
#include "client_utils.h"

using namespace std::chrono;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

class SharedQueue {
  private:
    std::mutex mutex;
    std::condition_variable cv;
    std::queue<Operation> queue;
    const int MaxQueueSize = 1000;

    ClientConfig &config;
  
  public:
    SharedQueue(ClientConfig &config): config(config) {};
    
    int enqueue() {
        std::unique_lock<std::mutex> lock(mutex);

        cv.wait(lock, [this] { return queue.size() < MaxQueueSize; });

        if (!moreOperationsExist(config)) {
            Operation op;
            op.__set_key("EOF");
            queue.push(op);

            lock.unlock();
            cv.notify_one();

            return 1;
        }

        queue.push(getOperation(config));

        lock.unlock();
        cv.notify_one();
        return 0;
    }

    Operation dequeue() {
        std::unique_lock<std::mutex> lock(mutex);

        cv.wait(lock, [this] { return !queue.empty(); });
        
        Operation data = queue.front();
        queue.pop();

        lock.unlock();
        cv.notify_one();
        return data;
    }
};

class DataHandler {
  private:
    SharedQueue &sharedQueue;

  public:
    DataHandler(SharedQueue& sharedQueue): sharedQueue(sharedQueue) {}

    void operator()() {
        while (true) {
            int enqueue_result = sharedQueue.enqueue();

            if (enqueue_result == 1) return;
        }
    }
};

class ClientRunner {
  private:
    SharedQueue &sharedQueue;
    std::vector<double> &latencies;

  public:
    ClientRunner(SharedQueue& sharedQueue, std::vector<double> &latencies): 
        sharedQueue(sharedQueue), latencies(latencies) {}

    void operator()() {
        while (true) {
            Operation data = sharedQueue.dequeue();

            if (data.key == "EOF") return;

            auto socket = std::make_shared<TSocket>(HOST_IP, HOST_PORT);
            auto transport = std::make_shared<TBufferedTransport>(socket);
            auto protocol = std::make_shared<TBinaryProtocol>(transport);
            RPCClient client(protocol);

            transport->open();

            auto start = high_resolution_clock::now();
            client.access(data);
            auto end = high_resolution_clock::now();
            latencies.push_back(
                duration_cast<microseconds>(end - start).count());
            
            transport->close();
        }
    }
};