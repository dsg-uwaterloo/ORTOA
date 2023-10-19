#include <fstream>
#include <iostream>
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

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

class ClientHandler {
 private:
	std::ifstream seed_data; 

 public:
	ClientHandler() = default;

	ClientHandler(std::string path) {
		seed_data.open(path);
		if (!seed_data.is_open()) {
        std::cerr << "File failed to open." << std::endl;
		}
	}

	void run(std::vector <float>& latencies) {
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
					client.access(val, op);
				}
			} 
			// If seed data does not exist, run client on random values
			else {
				for (int i = 0; i < 1000; ++i) {
					Operation op = genRandOperation();
					client.access(val, op);
				}
			}

			transport->close();
		} catch (TException& tx) {
			std::cout << "ERROR: " << tx.what() << std::endl;
  	}
	}
};


int main(int argc, char *argv[]) {
	std::vector<float> latencies;

	// If user runs client with path to seed data, init ClientHandler with seed
	ClientHandler client;
	if (argc >= 2){
		client = ClientHandler(argv[1]);
	}

	client.run(latencies);
}
