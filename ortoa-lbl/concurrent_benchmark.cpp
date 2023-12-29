#include <chrono>
#include <iostream>
#include <numeric>
#include <set>
#include <string>
#include <unordered_map>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "clientHelper.h"
#include "constants.h"
#include "gen-cpp/KV_RPC.h"
#include "gen-cpp/Operation_types.h"
#include "gen-cpp/Send_Op.h"
#include <thread>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace std::chrono;

std::atomic<int> accesses{0};
std::atomic<int> aborted{0};

std::atomic<long int> avg_encrypt{0};
std::atomic<long int> avg_round_trip{0};

void signal_callback_handler(int signum) {
  OpScureCleanup(DATA_FILE);
  delete pool;
  exit(signum);
}

// Operation parseOperation() {
//   Operation op;
//   std::string tmp;
//   std::cin >> tmp;
//   op.type = (tmp == "PUT") ? PUT : GET;
//   std::cin >> op.key;
//   if(op.type == PUT)
//     std::cin >> op.value;
//   return op;
// }

Operation randGenOperation() {
  float r = (float)rand() / RAND_MAX;
  Operation op;
  if (r < 0.5) {
    op.__set_op("PUT");
  } else {
    op.__set_op("GET");
  }
  int key = rand() % KEY_MAX;
  op.__set_key(std::string(std::to_string(key)));
  if (op.op == "GET") {
    char value[VALUE_SIZE];
    randombytes_buf(value, VALUE_SIZE);
    op.__set_value(std::string(value));
  }
  return op;
}

void handleOp(Operation op, std::string *_return, KV_RPCClient &client,
              std::vector<float> *stat) {
  std::string labels;

  if (!keySet.count(op.key)) {
    std::cerr << "No such key exists" << std::endl;

    if (op.op == "GET") {
      *_return = "";
      aborted++;
    } else {
      if (locks[op.key].exchange(false)) {
        auto start = high_resolution_clock::now();
        Entry createEntry = constructCreateEntry(op.key, op.value);
        auto encrypt_done = high_resolution_clock::now();
        valueSizes[op.key] = op.value.length();
        keySet.insert(op.key);

        client.create(createEntry);
        auto end = high_resolution_clock::now();
        accesses++;
        locks[op.key].exchange(true);
        avg_encrypt +=
            duration_cast<microseconds>(encrypt_done - start).count();
        avg_round_trip +=
            duration_cast<microseconds>(end - encrypt_done).count();
      } else {
        aborted++;
      }
    }
  } else {
    if (locks[op.key].exchange(false)) {
      if (op.op == "GET") {
        auto start = high_resolution_clock::now();
        Entry getEntry = constructGetEntry(op.key);
        auto encrypt_done = high_resolution_clock::now();
        client.access(labels, getEntry);
        auto end = high_resolution_clock::now();
        avg_encrypt +=
            duration_cast<microseconds>(encrypt_done - start).count();
        avg_round_trip +=
            duration_cast<microseconds>(end - encrypt_done).count();

        std::string value = readValueFromLabels(op.key, labels);
        // if (!value) {
        //   ++aborted;
        //   return;
        // }
        *_return = value;
        stat->push_back(duration_cast<microseconds>(end - start).count());
      } else {
        auto start = high_resolution_clock::now();

        Entry putEntry = constructPutEntry(op.key, op.value);
        auto encrypt_done = high_resolution_clock::now();
        valueSizes[op.key] = op.value.length();

        client.access(labels, putEntry);
        // if (!labels) {
        //   ++aborted;
        //   return;
        // }
        auto end = high_resolution_clock::now();
        avg_encrypt +=
            duration_cast<microseconds>(encrypt_done - start).count();
        avg_round_trip +=
            duration_cast<microseconds>(end - encrypt_done).count();
        stat->push_back(duration_cast<microseconds>(end - start).count());
      }
      accesses++;
      locks[op.key].exchange(true);
    } else {
      aborted++;
    }
  }

  // std::cerr << (op.type ? "PUT" : "GET") << " " << op.key << " " << op.value
  // << std::endl;
}

void clientThread(int i, std::vector<std::vector<float> *> *client_stats) {
  std::shared_ptr<TTransport> socket(new TSocket(SERVER_IP, SERVER_PORT));
  std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  KV_RPCClient client(protocol);

  Operation op;

  try {
    transport->open();
    float diff;
    std::vector<float> *avg_time = (*client_stats)[i];
    std::string key;
    std::string value;
    for (int j = 0; j < 100; j++) {
      Operation op = randGenOperation();
      std::string ret;
      handleOp(op, &ret, client, avg_time);
      // std::cout << get_times.back() << put_times.back()  << std::endl;
      // std::cerr << (op.type ? "PUT" : "GET") << " " << op.key << " " <<
      // op.value << std::endl;
    }
  } catch (TException &tx) {
    cout << "ERROR: " << tx.what() << endl;
  }

  transport->close();
}

int main() {
  OpScureSetup(DATA_FILE);
  pool = new BS::thread_pool(HW_THREADS);
  srand((unsigned)time(NULL));

  signal(SIGINT, signal_callback_handler);
  std::thread clients[NUM_CLIENTS];
  std::vector<std::vector<float> *> client_stats;
  auto begin = high_resolution_clock::now();
  for (int i = 0; i < NUM_CLIENTS; i++) {
    client_stats.push_back(new std::vector<float>);
    clients[i] = std::thread(clientThread, i, &client_stats);
  }
  for (int i = 0; i < NUM_CLIENTS; i++) {
    clients[i].join();
  }
  auto end = high_resolution_clock::now();

  int total_ops = 0;
  float total_time = 0;
  for (int i = 0; i < NUM_CLIENTS; i++) {
    total_ops += client_stats[i]->size();
    total_time +=
        std::accumulate(client_stats[i]->begin(), client_stats[i]->end(), 0.0);
    delete client_stats[i];
  }
  std::cout << "Completed " << total_ops << " in avg " << total_time / total_ops
            << std::endl;
  std::cout << "Total time "
            << duration_cast<microseconds>(end - begin).count() / 1000.0
            << " ms" << std::endl;
  std::cout << "Avg Encrypt " << avg_encrypt / total_ops << std::endl;
  std::cout << "Avg Access " << avg_round_trip / total_ops << std::endl;

  OpScureCleanup(DATA_FILE);
  delete pool;
}
