#include <unordered_map>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include "gen-cpp/Operation_types.h"
#include "gen-cpp/Send_Op.h"
#include <chrono>
#include <numeric>

#include <sodium.h>



using namespace std;
using namespace::chrono;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
#include <iostream>

#include "constants.h"




void signal_callback_handler(int signum) {
   exit(signum);
}

Operation parseOperation() {
  Operation op;
  std::string tmp;
  std::cin >> tmp;
  if(tmp == "EXIT"){
    op.__set_op("EXIT");
    return op;
  }
  op.__set_op((tmp == "PUT") ? "PUT" : "GET");
  std::cin >> tmp;
  op.__set_key(tmp);
  if(op.op == "PUT"){
    std::cin >> tmp;
    op.__set_value(tmp);
  }
  return op;
}

Operation randGenOperation(){
    float r = (float) rand()/RAND_MAX;
    Operation op;
    if(r < 0.5){
        op.__set_op("PUT");
    }
    else{
        op.__set_op("GET");
    }
    int key = rand()% KEY_MAX;
    op.__set_key(std::string(std::to_string(key)));
    if(op.op == "GET"){
        char value[VALUE_SIZE];
        randombytes_buf(value, VALUE_SIZE);
        op.__set_value(std::string(value));
    }
    return op;

}

void client(int i, vector<float>* latencies){
    std::shared_ptr<TTransport> socket(new TSocket(PROXY_IP, PROXY_PORT));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    Send_OpClient client(protocol);
    Operation op;

    try {
        transport->open();

        for(int i = 0; i < 100; i++){
            op = randGenOperation();
            std::string val;
            auto start = high_resolution_clock::now();
            client.access(val, op);
            auto end = high_resolution_clock::now();
            latencies->push_back(duration_cast<microseconds>(end - start).count());
            // if(op.op == "GET"){
            //     std::cout << val << std::endl;
            // }
        //std::cerr << (op.type ? "PUT" : "GET") << " " << op.key << " " << op.value << std::endl;
        }    
        std::cout << "Thread " << i << " done\n";

        transport->close();
    } catch (TException& tx) {
        cout << "ERROR: " << tx.what() << endl;
  }
}

// int main() {

//   signal(SIGINT, signal_callback_handler);

//   std::shared_ptr<TTransport> socket(new TSocket(PROXY_IP, PROXY_PORT));
//   std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
//   std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
//   Send_OpClient client(protocol);
//   Operation op;

//   try {
//     transport->open();

//     while(1) {
//       std::cerr << "> ";
//       op = parseOperation();
//       if(op.op == "EXIT"){
//         transport->close();
//         exit(0);
//       }
//       else{
//           std::string val;
//           client.access(val, op);
//           if(op.op == "GET"){
//               std::cout << val << std::endl;
//           }
//       }
      

//       //std::cerr << (op.type ? "PUT" : "GET") << " " << op.key << " " << op.value << std::endl;
//     }    

//     transport->close();
//   } catch (TException& tx) {
//     cout << "ERROR: " << tx.what() << endl;
//   }
// }

int main() {
    srand( (unsigned)time( NULL ) );
    std::thread t[NUM_CLIENTS];
    vector<float>* latencies[NUM_CLIENTS];
    for(int i = 0; i < NUM_CLIENTS; i++){
	    latencies[i] = new vector<float>;
    }
    auto start = high_resolution_clock::now();
    for(int i = 0; i < NUM_CLIENTS; i++){
        t[i] = std::thread(client, i, latencies[i]);
    }
    for(int i = 0; i < NUM_CLIENTS; i++){
        t[i].join();
    }
    auto end = high_resolution_clock::now();
    std::cout << "Finished in " << duration_cast<microseconds>(end - start).count() << " microseconds" << std::endl;
    vector<float> totals;
    for(int i = 0; i < NUM_CLIENTS; i++){
    	totals.push_back(std::accumulate(latencies[i]->begin(), latencies[i]->end(), 0));
    }

    std::cout << "Average Latency: " << std::accumulate(totals.begin(), totals.end(), 0.0) / (NUM_CLIENTS * 100) << endl;
}
