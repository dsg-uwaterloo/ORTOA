#include <iostream>
#include <sodium.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "../constants/network.h"
#include "../crypto/encryption_engine.h"
#include "../gen-cpp/Operation_types.h"
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
	if(op.op == OpType::GET) {
		char value[VALUE_SIZE];
		randombytes_buf(value, VALUE_SIZE);
		op.__set_value(std::string(value));
	}
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

// int main(int argc, char *argv[]) {
// 	encryption_engine encryption_engine_;
// 	redisCli rd;
// 	unsigned char cipher_text[4096];
// 	auto encLen = encryption_engine_.encryptNonDeterministic("30", cipher_text);
// 	string val((const char *)cipher_text, encLen);
// 	cout << "Encrypted value is: " << val << endl;
// 	rd.put("1", val);
// 	cout << "Redis get encrypted for key 1: " << rd.get("1") << endl;
//     cout << "Decrypted value for key 1: " << encryption_engine_.decryptNonDeterministic(rd.get("1")) << endl;
    
//     encLen = encryption_engine_.encryptNonDeterministic("20", cipher_text);
// 	string val1((const char *)cipher_text, encLen);
// 	rd.put("2", val1);
// 	cout << "Redis get encrypted for key 2: " << rd.get("2") << endl;
//     cout << "Decrypted value for key 1: " << encryption_engine_.decryptNonDeterministic(rd.get("2")) << endl;
// 	return 0;
// }