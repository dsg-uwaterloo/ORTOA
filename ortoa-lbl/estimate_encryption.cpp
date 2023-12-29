#include <set>
#include <string>
#include <iostream>
#include <unordered_map>
#include <numeric>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "gen-cpp/KV_RPC.h"
#include "clientHelper.h"


using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

enum OperationType {
  GET,
  PUT
};

struct Operation {
  OperationType type;
  std::string key;
  std::string value;
};

void signal_callback_handler(int signum) {
   OpScureCleanup(DATA_FILE);
   exit(signum);
}

Operation parseOperation() {
  Operation op;
  std::string tmp;
  std::cin >> tmp;
  op.type = (tmp == "PUT") ? PUT : GET;
  std::cin >> op.key;
  if(op.type == PUT)
    std::cin >> op.value;
  return op;
}

int main() {

  signal(SIGINT, signal_callback_handler);

//   std::shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
//   std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
//   std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
//   KV_RPCClient client(protocol);

  Operation op;

  try {
    // OpScureSetup(DATA_FILE);
    // transport->open();
    int num_bits = 8*250;
    clock_t begin_time, end_time;
    float diff;
    std::vector<float> get_encryption_time, put_encryption_time;
    char entry[num_bits/8];
    std::string str_entry;
    std::string value;
    for(int i = 0; i < 2000; i++) {
        bzero(entry, num_bits / 8);
        entry[i / 8] |= 1 << i % 8;
        str_entry = std::string(entry);
        value = "1";
        // std::string labels;
        begin_time = clock();
        Entry getEntry = constructGetEntry(str_entry);
        end_time = clock();
        diff = float(end_time - begin_time) / CLOCKS_PER_SEC;
        get_encryption_time.push_back(diff);

        value = "2";
        begin_time = clock();
        Entry putEntry = constructPutEntry(str_entry, value);
        // valueSizes[str_entry] = value.length();
        // client.access(labels, putEntry);
        end_time = clock();
        diff = float(end_time - begin_time) / CLOCKS_PER_SEC;
        put_encryption_time.push_back(diff);
    }    

    std::cout << "get_enc_times avg:" << 1.0 * std::accumulate(get_encryption_time.begin(), get_encryption_time.end(), 0.0) / get_encryption_time.size() << std::endl;
    std::cout << "put_enc_times avg:" << 1.0 * std::accumulate(put_encryption_time.begin(), put_encryption_time.end(), 0.0) / put_encryption_time.size() << std::endl;

    
  } catch (TException& tx) {
    cout << "ERROR: " << tx.what() << endl;
  }
}
