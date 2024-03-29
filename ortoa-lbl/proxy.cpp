// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "gen-cpp/Send_Op.h"
#include "clientHelper.h"
#include "gen-cpp/KV_RPC.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TSocket.h>

#include <atomic>
#include <chrono>

#include <algorithm>
#include <iostream>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace std::chrono;


#include "constants.h"


std::atomic<int> accesses{0};
std::atomic<int> aborted{0};

std::atomic<long int> avg_encrypt{0};
std::atomic<long int> avg_round_trip{0};


void handleOp(Operation op, std::string* _return, KV_RPCClient& client){
    std::string labels;

    if(!keySet.count(op.key)) {
        std::cerr << "No such key exists" << std::endl;

        if(op.op == "GET") {
          *_return = "";
          aborted++;
        } else {
          if(locks[op.key].exchange(false)){
            auto start = high_resolution_clock::now();
            Entry createEntry = constructCreateEntry(op.key, op.value);
            auto encrypt_done = high_resolution_clock::now();
            valueSizes[op.key] = op.value.length();
            keySet.insert(op.key);

            client.create(createEntry);
            auto end = high_resolution_clock::now();
            accesses++;
            locks[op.key].exchange(true);
            avg_encrypt += duration_cast<microseconds>(encrypt_done - start).count();
            avg_round_trip += duration_cast<microseconds>(end - encrypt_done).count();
          }
          else{
            aborted++;
          }
        }
      } else {
        if(locks[op.key].exchange(false)){
            if(op.op == "GET") {
            auto start = high_resolution_clock::now();
            Entry getEntry = constructGetEntry(op.key);
            auto encrypt_done = high_resolution_clock::now();
            client.access(labels, getEntry);
            auto end = high_resolution_clock::now();
            avg_encrypt += duration_cast<microseconds>(encrypt_done - start).count();
            avg_round_trip += duration_cast<microseconds>(end - encrypt_done).count();

            std::string value = readValueFromLabels(op.key, labels);
            *_return = value;
            } else {
            auto start = high_resolution_clock::now();

            Entry putEntry = constructPutEntry(op.key, op.value);
            auto encrypt_done = high_resolution_clock::now();
            valueSizes[op.key] = op.value.length();
            
            client.access(labels, putEntry);
            auto end = high_resolution_clock::now();
            avg_encrypt += duration_cast<microseconds>(encrypt_done - start).count();
            avg_round_trip += duration_cast<microseconds>(end - encrypt_done).count();
            }
            accesses++;
            locks[op.key].exchange(true);
        }
        else{
            aborted++;
        }
      }

      //std::cerr << (op.type ? "PUT" : "GET") << " " << op.key << " " << op.value << std::endl;  
}




class Send_OpHandler : virtual public Send_OpIf {
 public:

  Send_OpHandler() {
    
    OpScureSetup(DATA_FILE);
    pool = new BS::thread_pool(HW_THREADS);

  }

  void access(std::string& _return, const Operation& operation) {
    ::std::shared_ptr<TTransport> socket(new TSocket(SERVER_IP, SERVER_PORT));
    ::std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    ::std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    KV_RPCClient client(protocol);
    transport->open();

    Operation op;
    op.__set_op(operation.op);
    op.__set_key(operation.key);
    op.__set_value(operation.value);
    handleOp(op, &_return, client);
  }

};

void signal_callback_handler(int signum) {
   OpScureCleanup(DATA_FILE);
   std::cout << "Accesses: " << accesses << std::endl;
   std::cout << "Aborted: " << aborted << std::endl;
   std::cout << "Avg encrypt time: " << 1.0 * avg_encrypt / accesses << std::endl;
   std::cout << "Avg access time: " << 1.0 * avg_round_trip / accesses << std::endl;
   // Terminate program
   delete pool;
   exit(signum);
}

int main(int argc, char **argv) {
  signal(SIGINT, signal_callback_handler);
  int port = PROXY_PORT;
  ::std::shared_ptr<Send_OpHandler> handler(new Send_OpHandler());
  ::std::shared_ptr<TProcessor> processor(new Send_OpProcessor(handler));
  ::std::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  ::std::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  ::std::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  
 std::shared_ptr<apache::thrift::server::TServer> server;


  server.reset(
        new TThreadedServer(processor, serverTransport, transportFactory, protocolFactory));
  server->serve();

  return 0;
}

