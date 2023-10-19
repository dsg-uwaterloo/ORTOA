#include <chrono>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>
#include <sodium.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "client_utils.h"
#include "../constants/constants.h"
#include "../crypto/encryption_engine.h"
#include "../gen-cpp/RPC.h"
#include "../host/redis.h"

using namespace std::chrono;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

class ClientHandler {
 private:
	std::ifstream seed_data; 
	std::vector<float> latencies;

 public:
	ClientHandler() = default;

	ClientHandler(std::string path) {
		seed_data.open(path);
		if (!seed_data.is_open()) {
        std::cerr << "File failed to open." << std::endl;
		}
	}

	void run() {
		auto socket = std::make_shared<TSocket>(HOST_IP, HOST_PORT);
		auto transport = std::make_shared<TBufferedTransport>(socket);
		auto protocol = std::make_shared<TBinaryProtocol>(transport);
		RPCClient client(protocol);

		try {
			transport->open();

			std::string val;
			// If seed data exists, run the client with data
			if (seed_data.is_open()) {
				std::string line;
				while (std::getline(seed_data, line)) {
					Operation op = getSeedOperation(line);
					auto start = high_resolution_clock::now();
          client.access(val, op);
          auto end = high_resolution_clock::now();
					latencies.push_back(duration_cast<microseconds>(end - start).count());
				}
			} 
			// If seed data does not exist, run client on random values
			else {
				for (int i = 0; i < 1000; ++i) {
					Operation op = genRandOperation();
					auto start = high_resolution_clock::now();
					client.access(val, op);
					auto end = high_resolution_clock::now();
					latencies.push_back(duration_cast<microseconds>(end - start).count());
				}
			}

			transport->close();
		} catch (TException& tx) {
			std::cout << "ERROR: " << tx.what() << std::endl;
  	}
	}

	void getAveLatency() {
		std::cout << "[Client]: Data access complete, average latency: " << std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size() << " microseconds" << std::endl;
	}
};


int main(int argc, char *argv[]) {
  auto start = high_resolution_clock::now();

	// If user runs client with path to seed data, init ClientHandler with seed
	ClientHandler client;
	if (argc >= 2){
		client = ClientHandler(argv[1]);
	}

	client.run();
	client.getAveLatency();

	auto end = high_resolution_clock::now();
  std::cout << "[main]: Entire program finished in " << duration_cast<microseconds>(end - start).count() << " microseconds" << std::endl;
}
