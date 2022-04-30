#include "gen-cpp/KV_RPC.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include <cstdint>
#include <cassert>
#include <thread>
#include <algorithm>
#include <iostream>

#include <sodium.h>
#include "rocksdb/db.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

#include "constants.h"

#define NUM_THREADS 64

#define BLOCK_SIZE (VALUE_SIZE*4*(1 + crypto_secretbox_KEYBYTES))

#define CIPHERTEXT_LEN (crypto_secretbox_MACBYTES + crypto_secretbox_KEYBYTES + 1)

class KV_RPCHandler : virtual public KV_RPCIf {
 public:

  rocksdb::DB* db;

  KV_RPCHandler() {
    // Your initialization goes here

    rocksdb::Options options;
    options.create_if_missing = true;
    rocksdb::Status status = rocksdb::DB::Open(options, "db", &db);
    assert(status.ok());

  }

  void create(const Entry& entry) {
    // Your implementation goes here
    //printf("create %s\n", entry.keyName.c_str());
    //fflush(stdout);

    db->Put(rocksdb::WriteOptions(), entry.keyName, entry.encryptedLabelsA);

  }

  static void decryptPortion(int part, uint8_t* newKey, uint8_t* oldKey, uint8_t* A, uint8_t* B, uint8_t* C, uint8_t* D) {

    int partSize = (VALUE_SIZE / NUM_THREADS) + (VALUE_SIZE % NUM_THREADS != 0);
    int start = part * partSize;
    int limit = std::min((part + 1) * partSize, VALUE_SIZE);

    newKey += start * (1 + crypto_secretbox_KEYBYTES);
    oldKey += start * (1 + crypto_secretbox_KEYBYTES);

    A += start * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN);
    B += start * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN);
    C += start * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN);
    D += start * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN);

    uint8_t *nonce, *ciphertext;

    for(int i = start; i < limit; i++) {
      for(int j = 0; j < 4; j++) {

        int auxBits = oldKey[0] & 3;

        if(auxBits == 0){
          nonce = A;
          ciphertext = A + crypto_secretbox_NONCEBYTES;
        }
        else if(auxBits == 1){
          nonce = B;
          ciphertext = B + crypto_secretbox_NONCEBYTES;
        }
        else if(auxBits == 2){
          nonce = C;
          ciphertext = C + crypto_secretbox_NONCEBYTES;
        }
        else{
          nonce = D;
          ciphertext = D + crypto_secretbox_NONCEBYTES;
        }
        
        
        if (crypto_secretbox_open_easy(newKey, ciphertext, CIPHERTEXT_LEN, nonce, oldKey + 1) != 0) {
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
  }

  void access(std::string& _return, const Entry& entry) {
    // Your implementation goes here
    //printf("access %s\n", entry.keyName.c_str());

    std::string oldKeys;

    db->Get(rocksdb::ReadOptions(), entry.keyName, &oldKeys);

    uint8_t* oldKey = (uint8_t*) &oldKeys[0];

    uint8_t* labelListA = (uint8_t*) &entry.encryptedLabelsA[0];
    uint8_t* labelListB = (uint8_t*) &entry.encryptedLabelsB[0];
    uint8_t* labelListC = (uint8_t*) &entry.encryptedLabelsC[0];
    uint8_t* labelListD = (uint8_t*) &entry.encryptedLabelsD[0];



    _return.resize(BLOCK_SIZE);
    uint8_t* newKey = (uint8_t*) &_return[0];

    uint8_t* nonce;
    uint8_t* ciphertext;

    std::thread decryptionThreads[NUM_THREADS];

    for(int i = 0; i < NUM_THREADS; i++) {
      decryptionThreads[i] = std::thread(decryptPortion, i, newKey, oldKey, labelListA, labelListB, labelListC, labelListD);
    }

    for(int i = 0; i < NUM_THREADS; i++) {
      decryptionThreads[i].join();
    }

    db->Put(rocksdb::WriteOptions(), entry.keyName, _return);

  }

};

int main(int argc, char **argv) {
  int port = SERVER_PORT;
  ::std::shared_ptr<KV_RPCHandler> handler(new KV_RPCHandler());
  ::std::shared_ptr<TProcessor> processor(new KV_RPCProcessor(handler));
  ::std::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  ::std::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  ::std::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

