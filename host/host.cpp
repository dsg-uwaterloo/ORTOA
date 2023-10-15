// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <iostream>
#include "../constants/network.h"
#include "../gen-cpp/RPC.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

class RPCHandler : virtual public RPCIf {
 public:
  RPCHandler() {
    // Your initialization goes here
  }

  void access(std::string& _return, const Operation& operation) {
    // Your implementation goes here
    std::cout << operation.op << " " << operation.key << " " << operation.value << std::endl;
  }

};

int main(int argc, char **argv) {
  auto handler = std::make_shared<RPCHandler>();
  auto processor = std::make_shared<RPCProcessor>(handler);
  auto serverTransport = std::make_shared<TServerSocket>(HOST_PORT);
  auto transportFactory = std::make_shared<TBufferedTransportFactory>();
  auto protocolFactory = std::make_shared<TBinaryProtocolFactory>();

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

// #include <assert.h>
// #include <limits.h>
// #include <openenclave/host.h>
// #include <stdio.h>
// #include <sys/stat.h>
// #include <sys/types.h>
// #include <fstream>
// #include <iostream>
// #include <iterator>
// #include <vector>
// #include "../constants/shared.h"
// #include "redis.h"

// #include "ortoa_u.h"

// using namespace std;

// oe_enclave_t* enclave = NULL;

// bool check_simulate_opt(int* argc, const char* argv[])
// {
//     for (int i = 0; i < *argc; i++)
//     {
//         if (strcmp(argv[i], "--simulate") == 0)
//         {
//             cout << "Running in simulation mode" << endl;
//             memmove(&argv[i], &argv[i + 1], (*argc - i) * sizeof(char*));
//             (*argc)--;
//             return true;
//         }
//     }
//     return false;
// }

// int main(int argc, const char* argv[])
// {
//     oe_result_t result;
//     int ret = 0, res;
//     uint32_t flags = OE_ENCLAVE_FLAG_DEBUG;
//     redisCli rd;
//     unsigned char* out = NULL;
//     out = new unsigned char[4096];
//     size_t outLen;
//     string val;
//     string opConst = "1";
//     string updateVal;
//     char * copy;

//     if (check_simulate_opt(&argc, argv))
//     {
//         flags |= OE_ENCLAVE_FLAG_SIMULATE;
//     }

//     cout << "Host: enter main" << endl;
//     if (argc != 2)
//     {
//         cerr << "Usage: " << argv[0]
//              << " enclave_image_path [ --simulate  ]" << endl;
//         return 1;
//     }

//     cout << "Host: create enclave for image:" << argv[1] << endl;
//     result = oe_create_ortoa_enclave(
//         argv[1], OE_ENCLAVE_TYPE_SGX, flags, NULL, 0, &enclave);
//     if (result != OE_OK)
//     {
//         cerr << "oe_create_ortoa_enclave() failed with " << argv[0]
//              << " " << result << endl;
//         ret = 1;
//     }
//     val = rd.get("1");
//     cout << "Host: Redis get: " << val << " with len " << val.length() << endl;
//     result = access_data(enclave, opConst.c_str(), opConst.length(), val.c_str(), val.length(), out, &outLen);
//     if (result == OE_OK)
//     {
//         string updatedVal((const char *)out, outLen);
//         cout << "Host: Output of access_data " << updatedVal << " with len " << outLen << endl;
//         rd.reconnect();
//         rd.put("1", updatedVal);
//     }

// exit:
//     cout << "Host: terminate the enclave" << endl;
//     cout << "Host: Sample completed successfully." << endl;
//     if (enclave)
//         oe_terminate_enclave(enclave);
//     return ret;
// }