#include "gen-cpp/KV_RPC.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include "BS_thread_pool.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>

#include <chrono>

#include "rocksdb/db.h"
#include <sodium.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace std::chrono;

#include "constants.h"

#define BLOCK_SIZE (VALUE_SIZE * 4 * (1 + crypto_secretbox_KEYBYTES))

#define CIPHERTEXT_LEN                                                         \
  (crypto_secretbox_MACBYTES + crypto_secretbox_KEYBYTES + 1)

BS::thread_pool *pool;

std::atomic<long int> avg_access{0};
std::atomic<long int> access_count{0};
std::atomic<long int> avg_decrypt{0};

class KV_RPCHandler : virtual public KV_RPCIf {
public:
  rocksdb::DB *db;
  KV_RPCHandler() {
    // Your initialization goes here

    rocksdb::Options options;
    options.create_if_missing = true;
    rocksdb::Status status = rocksdb::DB::Open(options, "db", &db);
    assert(status.ok());
  }

  void create(const Entry &entry) {
    // Your implementation goes here
    // printf("create %s\n", entry.keyName.c_str());
    // fflush(stdout);

    db->Put(rocksdb::WriteOptions(), entry.keyName, entry.encryptedLabelsA);
  }

  static bool decryptPortion(int part, uint8_t *newKey, uint8_t *oldKey,
                             uint8_t *A, uint8_t *B, uint8_t *C, uint8_t *D) {

    int partSize = (VALUE_SIZE / SERVER_NUM_THREADS) +
                   (VALUE_SIZE % SERVER_NUM_THREADS != 0);
    int start = part * partSize;
    int limit = std::min((part + 1) * partSize, VALUE_SIZE);

    newKey += start * (1 + crypto_secretbox_KEYBYTES);
    oldKey += start * (1 + crypto_secretbox_KEYBYTES);

    A += start * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN);
    B += start * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN);
    C += start * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN);
    D += start * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN);

    uint8_t *nonce, *ciphertext;

    for (int i = start; i < limit; i++) {
      for (int j = 0; j < 4; j++) {

        int auxBits = oldKey[0] & 3;

        if (auxBits == 0) {
          nonce = A;
          ciphertext = A + crypto_secretbox_NONCEBYTES;
        } else if (auxBits == 1) {
          nonce = B;
          ciphertext = B + crypto_secretbox_NONCEBYTES;
        } else if (auxBits == 2) {
          nonce = C;
          ciphertext = C + crypto_secretbox_NONCEBYTES;
        } else {
          nonce = D;
          ciphertext = D + crypto_secretbox_NONCEBYTES;
        }

        if (crypto_secretbox_open_easy(newKey, ciphertext, CIPHERTEXT_LEN,
                                       nonce, oldKey + 1) != 0) {
          printf("RIP\n");
          fflush(stdout);
          exit(1);
        }

        A += crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN;
        B += crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN;
        C += crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN;
        D += crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN;
        newKey += crypto_secretbox_KEYBYTES + 1;
        oldKey += crypto_secretbox_KEYBYTES + 1;
      }
    }
    return true;
  }

  void access(std::string &_return, const Entry &entry) {
    auto access_begin = high_resolution_clock::now();
    // Your implementation goes here
    // printf("access %s\n", entry.keyName.c_str());

    // std::cout << "Got " << entry << std::endl;

    std::string oldKeys;

    db->Get(rocksdb::ReadOptions(), entry.keyName, &oldKeys);

    uint8_t *oldKey = (uint8_t *)&oldKeys[0];

    uint8_t *labelListA = (uint8_t *)&entry.encryptedLabelsA[0];
    uint8_t *labelListB = (uint8_t *)&entry.encryptedLabelsB[0];
    uint8_t *labelListC = (uint8_t *)&entry.encryptedLabelsC[0];
    uint8_t *labelListD = (uint8_t *)&entry.encryptedLabelsD[0];

    _return.resize(BLOCK_SIZE);
    uint8_t *newKey = (uint8_t *)&_return[0];

    std::future<bool> decryptionThreads[SERVER_NUM_THREADS];
    auto start = high_resolution_clock::now();

    for (int i = 0; i < SERVER_NUM_THREADS; i++) {
      decryptionThreads[i] =
          pool->submit(this->decryptPortion, i, newKey, oldKey, labelListA,
                       labelListB, labelListC, labelListD);
    }

    for (int i = 0; i < SERVER_NUM_THREADS; i++) {
      decryptionThreads[i].get();
    }

    auto stop = high_resolution_clock::now();
    avg_decrypt += duration_cast<microseconds>(stop - start).count();
    access_count++;

    db->Put(rocksdb::WriteOptions(), entry.keyName, _return);
    auto access_end = high_resolution_clock::now();
    avg_access +=
        duration_cast<microseconds>(access_end - access_begin).count();
  }
};

void signal_callback_handler(int signum) {
  std::cout << "Avg decrypt time: " << avg_decrypt * 1.0 / access_count
            << std::endl;
  std::cout << "Avg access time: " << avg_access * 1.0 / access_count
            << std::endl;
  exit(signum);
}

int main(int argc, char **argv) {
  signal(SIGINT, signal_callback_handler);
  int port = SERVER_PORT;
  ::std::shared_ptr<KV_RPCHandler> handler(new KV_RPCHandler());
  ::std::shared_ptr<TProcessor> processor(new KV_RPCProcessor(handler));
  ::std::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  ::std::shared_ptr<TTransportFactory> transportFactory(
      new TBufferedTransportFactory());
  ::std::shared_ptr<TProtocolFactory> protocolFactory(
      new TBinaryProtocolFactory());
  std::shared_ptr<apache::thrift::server::TServer> server;
  pool = new BS::thread_pool(HW_THREADS);

  server.reset(new TThreadedServer(processor, serverTransport, transportFactory,
                                   protocolFactory));
  server->serve();

  return 0;
}
