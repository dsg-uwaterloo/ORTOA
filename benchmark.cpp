#include <set>
#include <string>
#include <iostream>
#include <unordered_map>
#include <numeric>
#include <chrono>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "gen-cpp/KV_RPC.h"
#include "clientHelper.h"

#define DATA_FILE "/dsl/OpScure/OpScure.data"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace std::chrono;


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

  std::shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
  std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  KV_RPCClient client(protocol);

  Operation op;

  try {
    OpScureSetup(DATA_FILE);
    transport->open();
    float diff;
    std::vector<float> put_times, get_times;
    char entry[1000];
    std::string str_entry;
    std::string value;
    for(int i = 0; i < 8000; i++) {
        bzero(entry, 1000);
        entry[i / 8] |= 1 << i % 8;
        str_entry = std::string(entry);
        value = "1";
        Entry createEntry = constructCreateEntry(str_entry, value);
        valueSizes[str_entry] = value.length();
        keySet.insert(str_entry);
        client.create(createEntry);
        std::string labels;
        auto start = high_resolution_clock::now();
        Entry getEntry = constructGetEntry(str_entry);
        client.access(labels, getEntry);
        std::string value = readValueFromLabels(str_entry, labels);
        auto end = high_resolution_clock::now();
        diff = duration_cast<seconds>(stop - start);

        get_times.push_back(diff);
        value = "2";
        start = high_resolution_clock::now();
        Entry putEntry = constructPutEntry(str_entry, value);
        valueSizes[str_entry] = value.length();
        client.access(labels, putEntry);
        end = high_resolution_clock::now();
        diff = duration_cast<seconds>(stop - start);
        put_times.push_back(diff);
        // std::cout << get_times.back() << put_times.back()  << std::endl;
      //std::cerr << (op.type ? "PUT" : "GET") << " " << op.key << " " << op.value << std::endl;
    }    

    transport->close();
    std::cout << "get_times avg:" << 1.0 * std::accumulate(get_times.begin(), get_times.end(), 0.0) / get_times.size() << std::endl;
    std::cout << "put_times avg:" << 1.0 * std::accumulate(put_times.begin(), put_times.end(), 0.0) / put_times.size() << std::endl;

    
  } catch (TException& tx) {
    cout << "ERROR: " << tx.what() << endl;
  }
}
