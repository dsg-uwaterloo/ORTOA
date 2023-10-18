#include <iostream>
#include <sodium.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "../constants/constants.h"
#include "../crypto/encryption_engine.h"
#include "../gen-cpp/RPC.h"
#include "../host/redis.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

Operation genRandOperation() {
	float r = (float) rand() / RAND_MAX;
	int key = rand() % KEY_MAX;
  
	Operation op;
	op.__set_op(r < 0.5 ? OpType::PUT : OpType::GET);
  op.__set_key(std::to_string(key));

	std::string value;
	if (op.op == OpType::GET) {
		char rand_val[VALUE_SIZE];
		randombytes_buf(rand_val, VALUE_SIZE);
		value = std::string(rand_val);
	} else {
		int put_val = rand() % VAL_MAX;
		value = std::to_string(put_val);
	}

	encryption_engine engine;

	unsigned char* cipher_text = new unsigned char[4096];
	size_t out_len = (size_t) engine.encryptNonDeterministic(value, cipher_text);
	std::string updated_val((const char *) cipher_text, out_len);

	op.__set_value(updated_val);
	return op;
}

void client(std::vector <float>* latencies) {
	auto socket = std::make_shared<TSocket>(HOST_IP, HOST_PORT);
	auto transport = std::make_shared<TBufferedTransport>(socket);
	auto protocol = std::make_shared<TBinaryProtocol>(transport);
	RPCClient client(protocol);
	Operation op;

	try {
		transport->open();

		for (int i = 0; i < 10; ++i) {
			op = genRandOperation();
			std::string val;
			client.access(val, op);
		}
		
		transport->close();
	} catch (TException& tx) {
		std::cout << "ERROR: " << tx.what() << std::endl;
  }
}


int main(int argc, char *argv[]) {
	std::unique_ptr<std::vector<float>> latencies;
	client(latencies.get());
}
